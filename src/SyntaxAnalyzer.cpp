#include "SyntaxAnalyzer.h"
#include "Main.h"
#include "Math.h"
#include <fstream>
#include <iostream>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

bool debug = false; // Debug mode flag

// Global Variables
int nestingLevel = 0;         // Current code nesting level
int currentFunctionType = -1; // Type ID of the current function
int curPos = -1;              // Current index in the token vector
string CurrentFunction;       // Name of the current function

vector<Token *> v;      // Vector of parsed tokens
Token *cur;             // Current token being processed
stack<int> stateStack;         // Stack for parsing states
unordered_set<int> stateSet;   // Set for handling multiple parsing states

unordered_map<string, stack<TokenType>>
    names; // Map of variable names to types with scope handling
stack<pair<string, int>>
    lastNames; // Stack of variable names with their nesting levels
unordered_map<string, pair<int, bool>>
    functionHasReturn; // Map of functions, their return types, and return
                       // status

// RPN (Reverse Polish Notation) related
unordered_map<string, pair<vector<pair<int, string>>, vector<PToken>>>
    rpnMap; // Map of functions to their RPN expressions and arguments
unordered_map<string, stack<Variable>>
    rpnNames; // Scoped variable names for RPN expressions
stack<pair<string, int>>
    rpnLastNames; // Stack of RPN variable names with nesting levels

vector<int> posOfStart;    // Start positions of loops in RPN
vector<int> posOfEnd;      // End positions of loops in RPN
vector<int> posOfEndCnt;   // Counters for loop end positions
vector<int> posOfEndIf;    // End positions of if statements in RPN
vector<int> posOfEndCntIf; // Counters for if statement end positions

void resetSyntaxAnalyzerState() {
  debug = false;
  nestingLevel = 0;
  currentFunctionType = -1;
  curPos = -1;
  CurrentFunction.clear();
  v.clear();
  cur = nullptr;
  while (!stateStack.empty())
    stateStack.pop();
  stateSet.clear();
  names.clear();
  while (!lastNames.empty())
    lastNames.pop();
  functionHasReturn.clear();
  rpnMap.clear();
  rpnNames.clear();
  while (!rpnLastNames.empty())
    rpnLastNames.pop();
  posOfStart.clear();
  posOfEnd.clear();
  posOfEndCnt.clear();
  posOfEndIf.clear();
  posOfEndCntIf.clear();
}

// Function Declarations

// Returns an error message string, includes unexpected token or a custom error
// message, and optionally the line number
std::runtime_error err(const string &errString = "", bool showLine = true) {
  if (errString.empty()) {
    string s;
    s += "Unexpected token: (" + to_string(cur->type) + ") '" + cur->value +
         "'" + (showLine ? " (line " + to_string(cur->line) + ")" : "") + "\n";
    return std::runtime_error(s);
  } else {
    return std::runtime_error(
        "Error: " + errString +
        (showLine ? " (line " + to_string(cur->line) + ")" : "") + "\n");
  }
}

// Converts the current token to a string for debugging, including type, value,
// and line number
string tokenToString(Token *) { // Token TO String
  return "Debug: (" + to_string(cur->type) + ") '" + cur->value + "' (line " +
         to_string(cur->line) + ")\n";
}

// Prints the Reverse Polish Notation (RPN) for debugging, including the
// function name and its tokens
void debugRpn(const string &fun) {
  cout << "RPN DEBUG: '" << fun << "'\n";
  vector<PToken> tv = rpnMap[fun].second;
  for (int i = 0; i < tv.size(); ++i) {
    PToken t = tv[i];
    cout << i << "\t";
    if (t.type == PVariable || t.type == PFunction || t.type == PIO ||
        t.type == PType || t.type == PBinaryOperation ||
        t.type == PUnaryOperation || t.type == PSpecial) {
      cout << t.value;
    } else if (t.type == POperator) {
      cout << t.value;
      if (!t.args.empty()) {
        cout << "(" << t.args.back() << ")";
      }
    } else if (t.type == PIntValue) {
      cout << t.intValue;
    } else if (t.type == PDoubleValue) {
      cout << t.doubleValue;
    } else if (t.type == PStringValue) {
      cout << t.stringValue;
    } else if (t.type == PBoolValue) {
      cout << (t.boolValue ? "true" : "false");
    } else if (t.type == PNull) {
      cout << "NULL";
    }
    cout << "\n";
  }
  cout << "END OF RPN DEBUG\n";
}

// Converts a string representation of a type to its corresponding integer
// constant
int stringToType(const string &s) {
  if (s == "int")
    return TypeInt;
  if (s == "bool")
    return TypeBool;
  if (s == "string")
    return TypeString;
  if (s == "double")
    return TypeDouble;
  if (s == "null")
    return TypeNull;
  return -1;
}

// Converts an integer constant representing a type to its string representation
string typeToString(int type) {
  if (type == TypeInt)
    return "int";
  if (type == TypeBool)
    return "bool";
  if (type == TypeString)
    return "string";
  if (type == TypeDouble)
    return "double";
  if (type == TypeNull)
    return "null";
  return "~typeToString converting error~";
}

// Returns an error message string for type mismatch, includes current and
// expected types, and optionally the line number
std::runtime_error errType(int currentType, int expectedType,
                           bool showLine = true) {
  return std::runtime_error(
      "Incorrect Type: '" + typeToString(currentType) + "', expected '" +
      typeToString(expectedType) + "'" +
      (showLine ? " (line " + to_string(cur->line) + ")" : "") + "\n");
}

// Adds a state to the state stack and state set for tracking the current state
// during parsing
void addState(int state) {
  stateStack.push(state);
  stateSet.insert(state);
}

// Removes a state from the state stack and state set
void delState(int state) {
  stateStack.pop();
  stateSet.erase(state);
}

// Runs Syntax Analysis and execution
int runLexicalAnalysis(vector<Token *> tokens, bool debugFlag = false) {
  debug = debugFlag;
  v = std::move(tokens);

  nextToken();
  preprocessing();
  curPos = -1;
  nextToken();
  program();
  if (debug)
    cout << "STATUS : OK\n";
  return 0;
}

// Handles the preprocessing of global variables and functions, checks for the
// existence of the main function with correct signature
void preprocessing() {
  if (debug)
    cout << "F: preprocessing\n";
  globals();
  do {
    preprocessingFunction();
  } while (nextToken());
  if (names["main"].empty())
    throw err("Function 'main' is required", false);
  if (!names["main"].top().args.empty())
    throw err("Function 'main' must have no arguments", false);
  if (names["main"].top().type != TypeNull)
    throw err("Function 'main' must have type 'null'", false);
}

// Parses a function, including its return type, name, arguments, and body,
// handles nested states and return statement requirement
void function() {
  if (debug)
    cout << "F: function\n";
  if (!(cur->type == variableType || cur->type == functionType))
    throw err();
  currentFunctionType = stringToType(cur->value);
  nextToken();
  if (cur->type != name)
    throw err();
  string curName = cur->value;
  CurrentFunction = curName;
  nextToken();
  functionHasReturn[CurrentFunction] = {currentFunctionType, false};
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  arguments(curName, false);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != openingBrace)
    throw err();
  nextToken();
  addState(inFunction);
  block();
  delState(inFunction);
  if (cur->type != closingBrace)
    throw err();
  if (functionHasReturn[CurrentFunction].first != TypeNull &&
      !functionHasReturn[CurrentFunction].second) {
    throw err("Function '" + CurrentFunction +
              "' must have operator 'return' on main nesting level");
  }
  currentFunctionType = -1;
}

// Parses global import statements
void globals() {
  if (debug)
    cout << "F: globals\n";
  while (cur->value == "import") {
    nextToken();
    if (cur->type != stringConstant)
      throw err();
    nextToken();
    if (cur->type != semicolon)
      throw err();
    nextToken();
  }
}

// Advances to the next token in the token vector, returns 0 if the end is
// reached
int nextToken() {
  ++curPos;
  if (curPos >= v.size())
    return 0;
  cur = v[curPos];
  if (debug)
    cout << tokenToString(cur);
  return 1;
}

// Initiates the parsing of the program, including global imports and functions
void program() {
  if (debug)
    cout << "F: program\n";
  globals();
  functions();
}

// Parses multiple functions until the end of tokens is reached
void functions() {
  if (debug)
    cout << "F: functions\n";
  do {
    function();
  } while (nextToken());
}

// Preprocesses a function, checks for redeclaration and parses its signature
// and body
void preprocessingFunction() {
  if (debug)
    cout << "F: preprocessingFunction\n";
  if (!(cur->type == variableType || cur->type == functionType))
    throw err();
  int preprocessingCurrentFunctionType = stringToType(cur->value);
  nextToken();
  if (cur->type != name)
    throw err();
  string curName = cur->value;
  if (!names[curName].empty())
    throw err("Redeclaration function '" + curName + "'");
  names[curName].emplace(preprocessingCurrentFunctionType, -1, true);
  lastNames.emplace(curName, -1);
  nextToken();
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  arguments(curName);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != openingBrace)
    throw err();
  stack<bool> tmpStack;
  tmpStack.push(true);
  while (!tmpStack.empty() && curPos < v.size()) {
    nextToken();
    if (cur->type == openingBrace) {
      tmpStack.push(true);
    } else if (cur->type == closingBrace) {
      tmpStack.pop();
    }
  }
  if (cur->type != closingBrace)
    throw err();
  preprocessingCurrentFunctionType = -1;
}

// Parses the arguments of a function, checks for duplicate names and adds them
// to the names map
void arguments(const string &functionName, bool check) {
  set<string> tmpSet;
  if (debug)
    cout << "F: arguments\n";
  while (cur->type != closingBracket) {
    if (cur->type != variableType)
      throw err();
    int curType = stringToType(cur->value);
    if (check) {
      names[functionName].top().args.push_back(curType);
    }
    nextToken();
    if (cur->type != name)
      throw err();
    string curName = cur->value;
    if (check) {
      if (tmpSet.count(curName))
        throw err("Duplicate argument name '" + curName + "'");
      tmpSet.insert(curName);
    }
    if (!check) {
      if (!names[curName].empty() && names[curName].top().isFunction)
        throw err("'" + curName + "' is a Function");
      names[curName].emplace(curType, nestingLevel + 1);
      lastNames.emplace(curName, nestingLevel + 1);
      rpnMap[functionName].first.emplace_back(curType, curName);
    }
    nextToken();
    if (cur->type == closingBracket) {
      return;
    }
    if (cur->type != comma)
      throw err();
    nextToken();
  }
}

// Parses a block of statements, handles nesting levels and cleans up names at
// the end of the block
void block() {
  ++nestingLevel;
  if (debug)
    cout << "F: block\n";
  rpnMap[CurrentFunction].second.emplace_back(PSpecial, "levelup");
  do {
    operator_();
  } while (cur->type != closingBrace);
  --nestingLevel;
  rpnMap[CurrentFunction].second.emplace_back(PSpecial, "leveldown");
  while (!lastNames.empty() && lastNames.top().second > nestingLevel) {
    names[lastNames.top().first].pop();
    lastNames.pop();
  }
}

// Parses a single operator or statement, handles different types of operators
// and statements
void operator_() {
  if (debug)
    cout << "F: operator_\n";
  if (cur->type == name || cur->type == unaryMathOperator) {
    pair<int, vector<PToken>> p = expression();
    rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                          p.second.begin(), p.second.end());
    if (cur->type != semicolon)
      throw err();
    nextToken();
  } else if (cur->type == readwriteOperator) {
    operator_input_output();
  } else if (cur->type == sOperator) {
    operator_main();
  } else if (cur->type == variableType) {
    operator_variable_declaration();
  } else if (cur->type == closingBrace) {
    return;
  } else {
    throw err();
  }
}

// Parses main operators like while, for, if, return, continue, and break,
// directs to specific parsing functions
void operator_main() {
  if (debug)
    cout << "F: operator_main\n";
  if (cur->value == "while") {
    nextToken();
    operator_while();
  } else if (cur->value == "for") {
    nextToken();
    operator_for();
  } else if (cur->value == "if") {
    nextToken();
    operator_if();
  } else if (cur->value == "return") {
    operator_return();
  } else if (cur->value == "continue") {
    operator_continue();
  } else if (cur->value == "break") {
    operator_break();
  } else {
    throw err();
  }
}

// Handles the 'continue' operator within a loop
void operator_continue() {
  if (debug)
    cout << "F: operator_continue\n";
  if (stateSet.count(inCycle) == 0)
    throw err("Operator 'continue' is not in cycle");
  nextToken();
  if (cur->type != semicolon)
    throw err();
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  rpnMap[CurrentFunction].second.back().args.push_back(posOfStart.back());
  nextToken();
}

// Handles the 'break' operator within a loop
void operator_break() {
  if (debug)
    cout << "F: operator_break\n";
  if (stateSet.count(inCycle) == 0)
    throw err("Operator 'break' is not in cycle");
  nextToken();
  if (cur->type != semicolon)
    throw err();
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  posOfEnd.push_back(int(rpnMap[CurrentFunction].second.size()) - 1);
  ++posOfEndCnt.back();
  nextToken();
}

// Parses and handles the 'while' loop
void operator_while() {
  if (debug)
    cout << "F: operator_while\n";
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  pair<int, vector<PToken>> p = expression();
  int curType = p.first;
  if (curType != TypeBool)
    throw errType(curType, TypeBool);
  posOfStart.push_back(int(rpnMap[CurrentFunction].second.size()));
  rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                        p.second.begin(), p.second.end());
  rpnMap[CurrentFunction].second.emplace_back(POperator, "if");
  posOfEnd.push_back(int(rpnMap[CurrentFunction].second.size()) - 1);
  posOfEndCnt.push_back(1);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != openingBrace)
    throw err();
  nextToken();
  addState(inCycle);
  block();
  delState(inCycle);
  if (cur->type != closingBrace)
    throw err();
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  rpnMap[CurrentFunction].second.back().args.push_back(posOfStart.back());
  posOfStart.pop_back();
  for (int i = 0; i < posOfEndCnt.back(); ++i) {
    rpnMap[CurrentFunction].second[posOfEnd.back()].args.push_back(
        int(rpnMap[CurrentFunction].second.size()));
    posOfEnd.pop_back();
  }
  posOfEndCnt.pop_back();
  nextToken();
}

// Parses and handles the 'for' loop
void operator_for() {
  if (debug)
    cout << "F: operator_for\n";
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  addState(inFor1);
  if (cur->type == variableType) {
    operator_variable_declaration();
  } else {
    pair<int, vector<PToken>> p = expression();
    rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                          p.second.begin(), p.second.end());
  }
  delState(inFor1);
  if (cur->type != semicolon)
    throw err();
  nextToken();
  pair<int, vector<PToken>> cycleCondition = expression();
  int curType = cycleCondition.first;
  if (curType != TypeBool)
    throw errType(curType, TypeBool);
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  int posOfGoto = int(rpnMap[CurrentFunction].second.size()) - 1;
  if (cur->type != semicolon)
    throw err();
  nextToken();
  pair<int, vector<PToken>> cycleStep = expression();
  posOfStart.push_back(int(rpnMap[CurrentFunction].second.size()));
  rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                        cycleStep.second.begin(),
                                        cycleStep.second.end());
  rpnMap[CurrentFunction].second[posOfGoto].args.push_back(
      int(rpnMap[CurrentFunction].second.size()));
  rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                        cycleCondition.second.begin(),
                                        cycleCondition.second.end());
  rpnMap[CurrentFunction].second.emplace_back(POperator, "if");
  posOfEnd.push_back(int(rpnMap[CurrentFunction].second.size()) - 1);
  posOfEndCnt.push_back(1);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != openingBrace)
    throw err();
  nextToken();
  addState(inCycle);
  block();
  delState(inCycle);
  if (cur->type != closingBrace)
    throw err();
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  rpnMap[CurrentFunction].second.back().args.push_back(posOfStart.back());
  posOfStart.pop_back();
  for (int i = 0; i < posOfEndCnt.back(); ++i) {
    rpnMap[CurrentFunction].second[posOfEnd.back()].args.push_back(
        int(rpnMap[CurrentFunction].second.size()));
    posOfEnd.pop_back();
  }
  posOfEndCnt.pop_back();
  nextToken();
}

// Parses and handles the 'if' conditional statement
int operator_if(bool isRepeat) {
  if (debug)
    cout << "F: operator_if\n";
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  pair<int, vector<PToken>> p = expression();
  int curType = p.first;
  if (curType != TypeBool)
    throw errType(curType, TypeBool);
  if (!isRepeat)
    posOfEndCntIf.push_back(0);
  int answer = int(rpnMap[CurrentFunction].second.size());
  rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                        p.second.begin(), p.second.end());
  rpnMap[CurrentFunction].second.emplace_back(POperator, "if");
  int putHereEnd = int(rpnMap[CurrentFunction].second.size()) - 1;
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != openingBrace)
    throw err();
  nextToken();
  block();
  rpnMap[CurrentFunction].second.emplace_back(POperator, "goto");
  ++posOfEndCntIf.back();
  posOfEndIf.push_back(int(rpnMap[CurrentFunction].second.size()) - 1);
  if (cur->type != closingBrace)
    throw err();
  nextToken();
  if (cur->value != "else") {
    rpnMap[CurrentFunction].second[putHereEnd].args.push_back(
        int(rpnMap[CurrentFunction].second.size()));
    int sz = int(rpnMap[CurrentFunction].second.size());
    for (int i = 0; i < posOfEndCntIf.back(); ++i) {
      rpnMap[CurrentFunction].second[posOfEndIf.back()].args.push_back(sz);
      posOfEndIf.pop_back();
    }
    posOfEndCntIf.pop_back();
    return answer;
  }
  nextToken();
  if (cur->value == "if") {
    nextToken();
    int position = operator_if(true);
    rpnMap[CurrentFunction].second[putHereEnd].args.push_back(position);
  } else if (cur->value == "{") {
    rpnMap[CurrentFunction].second[putHereEnd].args.push_back(
        int(rpnMap[CurrentFunction].second.size()));
    nextToken();
    block();
    if (cur->type != closingBrace)
      throw err();
    int sz = int(rpnMap[CurrentFunction].second.size());
    for (int i = 0; i < posOfEndCntIf.back(); ++i) {
      rpnMap[CurrentFunction].second[posOfEndIf.back()].args.push_back(sz);
      posOfEndIf.pop_back();
    }
    posOfEndCntIf.pop_back();
    nextToken();
  }
  return answer;
}

// Handles the 'return' operator within a function
void operator_return() {
  if (debug)
    cout << "F: operator_return\n";
  nextToken();
  if (stateSet.count(inFunction) == 0)
    throw err();
  int returnHasArgs = 0;
  if (cur->type != semicolon) {
    returnHasArgs = 1;
    pair<int, vector<PToken>> p = expression();
    int curType = p.first;
    if (curType != currentFunctionType)
      throw errType(curType, currentFunctionType);
    if (cur->type != semicolon)
      throw err();
    rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                          p.second.begin(), p.second.end());
  } else {
    if (currentFunctionType != TypeNull) {
      throw errType(TypeNull, currentFunctionType);
    }
  }
  rpnMap[CurrentFunction].second.emplace_back(POperator, "return");
  rpnMap[CurrentFunction].second.back().args.push_back(returnHasArgs);
  if (nestingLevel == 1) {
    functionHasReturn[CurrentFunction].second = true;
  }
  nextToken();
}

// Handles input and output operations, directing to the appropriate function
// for handling 'read' or 'write' operations
void operator_input_output() {
  if (debug)
    cout << "F: operator_input_output\n";
  if (cur->value == "read") {
    nextToken();
    operator_io_read();
  } else if (cur->value == "write") {
    nextToken();
    operator_io_write();
  }
}

// Handles variable assignment operations
void operator_assignment(int varType, const string &varName) {
  if (debug)
    cout << "F: operator_assignment\n";
  if (cur->type != assignmentOperator)
    throw err();
  nextToken();
  pair<int, vector<PToken>> p = expression();
  int curExpType = p.first;
  if (curExpType != varType)
    throw errType(curExpType, varType);
  if (cur->type != semicolon)
    throw err();
  if (stateSet.count(inFor1) == 0)
    nextToken();
  rpnMap[CurrentFunction].second.emplace_back(PVariable, varName);
  rpnMap[CurrentFunction].second.insert(rpnMap[CurrentFunction].second.end(),
                                        p.second.begin(), p.second.end());
  rpnMap[CurrentFunction].second.emplace_back(PBinaryOperation, "=");
}

// Parses expressions, handling various operators
pair<int, vector<PToken>> expression() {
  if (debug)
    cout << "F: expression\n";
  static const unordered_map<string, int> priority = {
      {"++", 3}, {"!", 3},    {"*", 5},   {"/", 5},  {"%", 5},
      {"+", 6},  {"-", 6},    {"<", 8},   {">", 8},  {"<=", 8},
      {">=", 8}, {"==", 9},   {"!=", 9},  {"and", 12},
      {"xor", 13},{"or", 14}, {"**", 4},  {"=", 15}, {"+=", 15},
      {"-=", 15},{"*=", 15},  {"/=", 15}, {"%=", 15}};
  const auto priorityOf = [&](const string &op) {
    return priority.at(op);
  };
  stack<Token *> signs;

  vector<Token *> ans;
  vector<PToken> expressionInPolishNotation;

  bool afterOpeningBracket = false;
  int afterOpBracket = 0;
  bool mustBeObject = false;
  bool mustBeVar = false;
  bool afterVariable = false;

  int inFunction = 0;

  while ((cur->type != closingBracket || afterOpBracket > 0) &&
         cur->type != semicolon && (cur->type != comma || afterOpBracket > 0)) {
    if (cur->type == integerNumber || cur->type == doubleNumber ||
        cur->type == stringConstant || cur->type == logicalConstant) {
      ans.push_back(cur);
      nextToken();
      afterVariable = true;
      afterOpeningBracket = false;
    } else if (cur->type == name) {
      afterOpeningBracket = false;
      Token *q = cur;

      nextToken();
      if (cur->type == openingBracket) {
        q->isFunction = true;
        ++inFunction;
        ++afterOpBracket;
        signs.push(q);
        signs.push(cur);
        nextToken();
      } else {
        afterVariable = true;
        ans.push_back(q);
      }
    } else if (cur->type == comma) {
      if (!inFunction)
        throw err();
      while (!signs.empty() && signs.top()->type != openingBracket) {
        ans.push_back(signs.top());
        signs.pop();
      }
      if (signs.empty())
        throw err();
      nextToken();
    } else if ((cur->type == unaryMathOperator) ||
               (cur->type == logicalOperator && cur->value == "!")) {
      if (afterVariable)
        throw err("Postfix increment/decrement is not included");
      signs.push(cur);
      nextToken();
    } else if (cur->type == binaryMathOperator ||
               cur->type == comparisonOperator ||
               cur->type == assignmentOperator ||
               cur->type == logicalOperator) {
      afterVariable = false;
      const int currentPriority = priorityOf(cur->value);
      if (cur->value == "=" || cur->value == "**") {
        while (!signs.empty() && signs.top()->type != openingBracket &&
               ((signs.top()->type == unaryMathOperator) ||
                (signs.top()->type == logicalOperator &&
                 signs.top()->value == "!") ||
                priorityOf(signs.top()->value) < currentPriority)) {
          ans.push_back(signs.top());
          signs.pop();
        }
      } else {
        while (!signs.empty() && signs.top()->type != openingBracket &&
               ((signs.top()->type == unaryMathOperator) ||
                (signs.top()->type == logicalOperator &&
                 signs.top()->value == "!") ||
                priorityOf(signs.top()->value) <= currentPriority)) {
          ans.push_back(signs.top());
          signs.pop();
        }
      }
      signs.push(cur);
      nextToken();
    } else if (cur->type == openingBracket) {
      ++afterOpBracket;
      afterOpeningBracket = true;
      signs.push(cur);
      nextToken();
    } else if (cur->type == closingBracket) {
      --afterOpBracket;
      if (afterOpeningBracket)
        throw err("Empty brackets");
      while (!signs.empty() && signs.top()->type != openingBracket) {
        ans.push_back(signs.top());
        signs.pop();
      }
      if (signs.empty()) {
        throw err();
      } else {
        signs.pop();
        if (!signs.empty() && signs.top()->type == name) {
          ans.push_back(signs.top());
          signs.pop();
          --inFunction;
        }
      }
      nextToken();
    } else
      throw err();
  }
  while (!signs.empty()) {
    if (signs.top()->type == openingBracket)
      throw err();
    ans.push_back(signs.top());
    signs.pop();
  }
  for (auto tkn : ans) {
    if (tkn->type == integerNumber) {
      expressionInPolishNotation.emplace_back(PIntValue, tkn->value);
      try {
        expressionInPolishNotation[(int)expressionInPolishNotation.size() - 1]
            .intValue = stoi(tkn->value);
      } catch (...) {
        throw err("Number in input is too big");
      }
    } else if (tkn->type == doubleNumber) {
      expressionInPolishNotation.emplace_back(PDoubleValue, tkn->value);
      expressionInPolishNotation[(int)expressionInPolishNotation.size() - 1]
          .doubleValue = stod(tkn->value);
    } else if (tkn->type == stringConstant) {
      expressionInPolishNotation.emplace_back(PStringValue, tkn->value);
      expressionInPolishNotation[(int)expressionInPolishNotation.size() - 1]
          .stringValue = tkn->value;
    } else if (tkn->type == logicalConstant) {
      expressionInPolishNotation.emplace_back(PBoolValue, tkn->value);
      expressionInPolishNotation[(int)expressionInPolishNotation.size() - 1]
          .boolValue = (tkn->value == "true");
    } else if (tkn->type == name && tkn->isFunction) {
      expressionInPolishNotation.emplace_back(PFunction, tkn->value);
    } else if (tkn->type == name) {
      expressionInPolishNotation.emplace_back(PVariable, tkn->value);
    } else if (tkn->type == unaryMathOperator ||
               (tkn->type == logicalOperator && tkn->value == "!")) {
      expressionInPolishNotation.emplace_back(PUnaryOperation, tkn->value);
    } else if (tkn->type == binaryMathOperator ||
               tkn->type == logicalOperator ||
               tkn->type == comparisonOperator ||
               tkn->type == assignmentOperator) {
      expressionInPolishNotation.emplace_back(PBinaryOperation, tkn->value);
    } else
      throw err();
  }
  if (ans.empty())
    return {TypeNull, expressionInPolishNotation};

  stack<expressionElement *> exec;
  auto ptr = names.end();
  int counter = 0;
  if (debug) {
    for (auto &tkn : ans) {
      cout << tkn->type << " " << tkn->value << " " << endl;
    }
  }

  for (auto &an : ans) {
    if (an->type == name) {
      if (an->isFunction) {
        if (an->value == "main")
          throw err("Can't call function 'main'");
        ptr = names.find(an->value);
        if (ptr == names.end() || ptr->second.empty() ||
            !ptr->second.top().isFunction)
          throw err("Function '" + an->value + "' is not declared");
        counter = int(ptr->second.top().args.size()) - 1;
        while (counter >= 0 && !exec.empty()) {
          if (exec.top()->type != ptr->second.top().args[counter])
            throw err("Incorrect arguments of function");
          delete exec.top();
          exec.pop();
          --counter;
        }
        if (counter > -1)
          throw err("Incorrect number of arguments of function");
        exec.push(new expressionElement(ptr->second.top().type));
      } else {
        ptr = names.find(an->value);
        if (ptr == names.end() || ptr->second.empty())
          throw err("Variable '" + an->value + "' is not declared");
        else if (ptr->second.top().isFunction)
          throw err("Name '" + an->value + "' is already engaged by function.");
        exec.push(new expressionElement(ptr->second.top().type));
        exec.top()->isSimpleVariable = true;
      }
    } else if (an->type == integerNumber) {
      exec.push(new expressionElement(3));
    } else if (an->type == doubleNumber) {
      exec.push(new expressionElement(4));
    } else if (an->type == stringConstant) {
      exec.push(new expressionElement(2));
    } else if (an->type == logicalConstant) {
      exec.push(new expressionElement(1));
    } else if (an->type == unaryMathOperator) {
      if (exec.empty())
        throw err("Syntax error");
      if (an->value == "++" || an->value == "--") {
        if (!exec.top()->isSimpleVariable)
          throw err("Increment/decrement operations cannot be applied to "
                    "non-variables");
      }
      if (!(exec.top()->type == TypeInt || exec.top()->type == TypeDouble)) {
        throw err(
            "Unary arithmetic operations cannot be applied to non-numbers");
      }
      exec.top()->isSimpleVariable = false;

    } else if (an->type == logicalOperator && an->value == "!") {
      if (exec.empty())
        throw err("Syntax error");
      if (exec.top()->type != TypeBool) {
        throw err("Logic operations cannot be applied to non-bool types");
      }
      exec.top()->isSimpleVariable = false;
    } else if (an->type == logicalOperator) {
      expressionElement *sec, *fir;
      if (exec.empty())
        throw err("Syntax error");
      sec = exec.top();
      exec.pop();
      if (exec.empty())
        throw err("Syntax error");
      fir = exec.top();
      if (sec->type != TypeBool || fir->type != TypeBool) {
        throw err("Logic operations cannot be applied to non-bool types");
        // type mismatch in a boolean expression
      }
      exec.top()->isSimpleVariable = false;
      delete sec;
    } else if (an->type == binaryMathOperator) {
      expressionElement *sec, *fir;
      if (exec.empty())
        throw err("Syntax error");
      sec = exec.top();
      exec.pop();
      if (exec.empty())
        throw err("Syntax error");
      fir = exec.top();
      if (an->value == "+") {
        if (!((sec->type == TypeInt && fir->type == TypeInt) ||
              (sec->type == TypeDouble && fir->type == TypeDouble) ||
              (sec->type == TypeString && fir->type == TypeString)))
          throw err(
              "Operation '+' can be applied only to similar integer, double or "
              "string types"); // type mismatch in mathematical/string
                               // expression
      } else if (!((sec->type == TypeInt && fir->type == TypeInt) ||
                   (sec->type == TypeDouble && fir->type == TypeDouble))) {
        throw err(
            "Arithmetic operations can be applied only to similar integer or "
            "double types"); // type mismatch in mathematical expression
      }
      exec.top()->isSimpleVariable = false;
      delete sec;
    } else if (an->type == comparisonOperator) {
      expressionElement *sec, *fir;
      if (exec.empty())
        throw err("Syntax error");
      sec = exec.top();
      exec.pop();
      if (exec.empty())
        throw err("Syntax error");
      fir = exec.top();
      if (debug)
        cout << fir->type << " " << sec->type << endl;
      if (an->value == "==" || an->value == "!=") {
        if (!((sec->type == TypeInt && fir->type == TypeInt) ||
              (sec->type == TypeDouble && fir->type == TypeDouble) ||
              (sec->type == TypeString && fir->type == TypeString) ||
              (sec->type == TypeBool && fir->type == TypeBool))) {
          throw err("Equal/Not Equal operations can be applied only to similar "
                    "non-null types"); // type mismatch in mathematical/string
                                       // expression
        }
      } else if (!((sec->type == TypeInt && fir->type == TypeInt) ||
                   (sec->type == TypeDouble && fir->type == TypeDouble) ||
                   (sec->type == TypeString && fir->type == TypeString))) {
        throw err(
            "Comparison operations can be applied only to similar non-null, "
            "non-bool types"); // type mismatch in mathematical/string
                               // expression
      }
      delete sec;
      exec.top()->isSimpleVariable = false;
      exec.top()->type = TypeBool;
    } else if (an->type == assignmentOperator) {
      expressionElement *sec, *fir;
      if (exec.empty())
        throw err("Syntax error");
      sec = exec.top();
      exec.pop();
      if (exec.empty())
        throw err("Syntax error");
      fir = exec.top();
      if (!(fir->isSimpleVariable) || fir->type != sec->type)
        throw err("Assignment can be applied only to variables");
      delete sec;
      fir->isSimpleVariable = false;
    } else
      throw err("Trolling occurred...");
  }
  if (exec.size() != 1)
    throw err("Syntax error");
  int q = exec.top()->type;
  delete exec.top();

  return {q, expressionInPolishNotation};
}

// Counts and processes the arguments for a given function call
int arguments_to_call(const string &functionName) {
  if (debug)
    cout << "F: arguments_to_call\n";
  int k = 0;
  while (cur->type != closingBracket) {
    ++k;
    if (functionName == "read") {
      if (cur->type != name)
        throw err();
      if (names[cur->value].empty())
        throw err("Variable '" + cur->value + "' is not declared");
      if (names[cur->value].top().isFunction)
        throw err("'" + cur->value + "' is a Function");
      rpnMap[CurrentFunction].second.emplace_back(PVariable, cur->value);
      nextToken();
    } else {
      pair<int, vector<PToken>> p = expression();
      int curType = p.first;
      if (curType == TypeNull)
        throw err("Cannot write NULL");
      rpnMap[CurrentFunction].second.insert(
          rpnMap[CurrentFunction].second.end(), p.second.begin(),
          p.second.end());
    }
    if (cur->type == closingBracket)
      break;
    if (cur->type != comma)
      throw err();
    nextToken();
  }
  return k;
}

// Handles the "read" IO operation
void operator_io_read() {
  if (debug)
    cout << "F: operator_io_read\n";
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  int argsCnt = arguments_to_call("read");
  if (argsCnt == 0) {
    throw err("Function 'read' must have at least one argument");
  }
  rpnMap[CurrentFunction].second.emplace_back(PIO, "read");
  rpnMap[CurrentFunction].second.back().args.push_back(argsCnt);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != semicolon)
    throw err();
  nextToken();
}

// Handles the "write" IO operation
void operator_io_write() {
  if (debug)
    cout << "F: operator_io_write\n";
  if (cur->type != openingBracket)
    throw err();
  nextToken();
  int argsCnt = arguments_to_call("write");
  rpnMap[CurrentFunction].second.emplace_back(PIO, "write");
  rpnMap[CurrentFunction].second.back().args.push_back(argsCnt);
  if (cur->type != closingBracket)
    throw err();
  nextToken();
  if (cur->type != semicolon)
    throw err();
  nextToken();
}

// Handles variable declaration and optional initialization
void operator_variable_declaration() {
  if (debug)
    cout << "F: operator_variable_declaration\n";
  if (cur->type != variableType)
    throw err();
  int curVarType = stringToType(cur->value);
  nextToken();
  if (cur->type != name)
    throw err();
  string curName = cur->value;
  nextToken();
  if (cur->type == assignmentOperator) {
    rpnMap[CurrentFunction].second.emplace_back(PVariable, curName);
    rpnMap[CurrentFunction].second.emplace_back(PType,
                                                typeToString(curVarType));
    operator_assignment(curVarType, curName);
    if (!names[curName].empty() &&
        (names[curName].top().level == nestingLevel ||
         names[curName].top().isFunction))
      throw err("Name '" + curName + "' is already used");
    if (stateSet.count(inFor1)) {
      names[curName].emplace(curVarType, nestingLevel + 1);
      lastNames.emplace(curName, nestingLevel + 1);
    } else {
      names[curName].emplace(curVarType, nestingLevel);
      lastNames.emplace(curName, nestingLevel);
    }

  } else if (cur->type == semicolon) {
    if (!names[curName].empty() &&
        (names[curName].top().level == nestingLevel ||
         names[curName].top().isFunction))
      throw err("Name '" + curName + "' is already used");
    if (stateSet.count(inFor1)) {
      names[curName].emplace(curVarType, nestingLevel + 1);
      lastNames.emplace(curName, nestingLevel + 1);
    } else {
      names[curName].emplace(curVarType, nestingLevel);
      lastNames.emplace(curName, nestingLevel);
    }

    if (stateSet.count(inFor1) == 0)
      nextToken();
    rpnMap[CurrentFunction].second.emplace_back(PVariable, curName);
    rpnMap[CurrentFunction].second.emplace_back(PType,
                                                typeToString(curVarType));
  } else {
    throw err();
  }
}

// Executes a given function with specified arguments and nesting level
PToken exec(const string &functionName, const vector<PToken> &args,
            int nestLvl) { // args contains ONLY VALUES (P...Value)
  if (debug)
    cout << "exec: " << functionName << ", args.size = " << args.size() << "\n";

  const auto functionIt = rpnMap.find(functionName);
  if (functionIt == rpnMap.end()) {
    throw runtime_error("Function '" + functionName + "' is not declared");
  }
  const auto &functionData = functionIt->second;
  const auto &functionArgs = functionData.first;
  const auto &curRpn = functionData.second;

  int startLvl = nestLvl;
  for (size_t i = 0; i < args.size(); ++i) {
    const auto &argName = functionArgs[i].second;
    auto &argStack = rpnNames[argName];
    argStack.emplace(functionArgs[i].first);
    rpnLastNames.emplace(argName, nestLvl);
    const auto &arg = args[i];
    Variable &target = argStack.top();
    if (arg.type == PIntValue) {
      target.intValue = arg.intValue;
    } else if (arg.type == PDoubleValue) {
      target.doubleValue = arg.doubleValue;
    } else if (arg.type == PStringValue) {
      target.stringValue = arg.stringValue;
    } else if (arg.type == PBoolValue) {
      target.boolValue = arg.boolValue;
    }
  }
  if (debug)
    cout << "Args: ok\n";
  if (debug)
    debugRpn(functionName);
  stack<PToken> s;
  size_t i = 0;
  while (i < curRpn.size()) {
    if (debug)
      cout << "CURRENT NESTLVL: " << nestLvl << "\n";
    PToken tkn = curRpn[i];
    if (tkn.type == PVariable || tkn.type == PIntValue ||
        tkn.type == PDoubleValue || tkn.type == PStringValue ||
        tkn.type == PBoolValue) {
      if (debug)
        cout << "POperand: " << tkn.type << "\n";
      s.push(tkn);
    } else if (tkn.type == PType) {
      if (debug)
        cout << "PType: " << tkn.value << "\n";
      PToken t = s.top();
      s.pop();
      rpnNames[t.value].emplace(stringToType(tkn.value));
      rpnLastNames.emplace(t.value, nestLvl);
    } else if (tkn.type == PSpecial) {
      if (tkn.value == "levelup") {
        ++nestLvl;
      } else if (tkn.value == "leveldown") {
        while (!rpnLastNames.empty() && rpnLastNames.top().second >= nestLvl) {
          rpnNames[rpnLastNames.top().first].pop();
          rpnLastNames.pop();
        }
        --nestLvl;
      }
    } else if (tkn.type == POperator) {
      if (debug)
        cout << "POperator: " << tkn.value << "\n";
      if (tkn.value == "goto") {
        i = static_cast<size_t>(tkn.args.back());
        continue;
      } else if (tkn.value == "if") {
        PToken t = s.top();
        s.pop();
        if (t.boolValue) {
          ++i;
        } else {
          i = static_cast<size_t>(tkn.args.back());
        }
        continue;
      } else if (tkn.value == "return") {
        if (tkn.args.back()) {
          PToken t = s.top();
          s.pop();
          if (t.type == PVariable) {
            Variable curVariable = rpnNames[t.value].top();
            int varType = curVariable.type;
            if (varType == TypeInt) {
              t.type = PIntValue;
              t.intValue = curVariable.intValue;
            } else if (varType == TypeDouble) {
              t.type = PDoubleValue;
              t.doubleValue = curVariable.doubleValue;
            } else if (varType == TypeString) {
              t.type = PStringValue;
              t.stringValue = curVariable.stringValue;
            } else if (varType == TypeBool) {
              t.type = PBoolValue;
              t.boolValue = curVariable.boolValue;
            }
          }
          while (!rpnLastNames.empty() &&
                 rpnLastNames.top().second >= startLvl) {
            rpnNames[rpnLastNames.top().first].pop();
            rpnLastNames.pop();
          }
          return t;
        } else {
          while (!rpnLastNames.empty() &&
                 rpnLastNames.top().second >= startLvl) {
            rpnNames[rpnLastNames.top().first].pop();
            rpnLastNames.pop();
          }
          return {};
        }
      }
    } else if (tkn.type == PUnaryOperation) {
      if (debug)
        cout << "PUnaryOperation: " << tkn.value << "\n";
      PToken t = s.top();
      s.pop();
      PToken newT = PToken();
      if (tkn.value == "!") {
        newT.type = PBoolValue;
        if (t.type == PVariable) {
          newT.boolValue = !rpnNames[t.value].top().boolValue;
        } else if (t.type == PBoolValue) {
          newT.boolValue = !t.boolValue;
        }
        s.push(newT);
      } else if (tkn.value == "-") {
        if (t.type == PVariable) {
          if (rpnNames[t.value].top().type == TypeDouble) {
            newT.type = PDoubleValue;
            newT.doubleValue = -rpnNames[t.value].top().doubleValue;
          } else if (rpnNames[t.value].top().type == TypeInt) {
            newT.type = PIntValue;
            newT.intValue = -rpnNames[t.value].top().intValue;
          }
        } else if (t.type == PDoubleValue) {
          newT.type = PDoubleValue;
          newT.doubleValue = -t.doubleValue;
        } else if (t.type == PIntValue) {
          newT.type = PIntValue;
          newT.intValue = -t.intValue;
        }
        s.push(newT);
      } else if (tkn.value == "++") {
        // PVariable
        if (rpnNames[t.value].top().type == TypeDouble) {
          newT.type = PDoubleValue;
          newT.doubleValue = rpnNames[t.value].top().doubleValue =
              rpnNames[t.value].top().doubleValue + 1;
        } else if (rpnNames[t.value].top().type == TypeInt) {
          newT.type = PIntValue;
          newT.intValue = rpnNames[t.value].top().intValue =
              rpnNames[t.value].top().intValue + 1;
        }
        s.push(newT);
      } else if (tkn.value == "--") {
        // PVariable
        if (rpnNames[t.value].top().type == TypeDouble) {
          newT.type = PDoubleValue;
          newT.doubleValue = rpnNames[t.value].top().doubleValue =
              rpnNames[t.value].top().doubleValue - 1;
        } else if (rpnNames[t.value].top().type == TypeInt) {
          newT.type = PIntValue;
          newT.intValue = rpnNames[t.value].top().intValue =
              rpnNames[t.value].top().intValue - 1;
        }
        s.push(newT);
      }
    } else if (tkn.type == PBinaryOperation) {
      if (debug)
        cout << "PBinaryOperation: " << tkn.value << "\n";
      PToken t2 = s.top();
      s.pop();
      PToken t1 = s.top();
      s.pop();
      PToken newT = PToken();
      if (tkn.value == "=") {
        // PVariable
        // P...Value or PVariable
        if (rpnNames[t1.value].top().type == TypeInt) {
          newT.type = PIntValue;
          newT.intValue = rpnNames[t1.value].top().intValue =
              (t2.type == PVariable ? rpnNames[t2.value].top().intValue
                                    : t2.intValue);
        } else if (rpnNames[t1.value].top().type == TypeDouble) {
          newT.type = PDoubleValue;
          newT.doubleValue = rpnNames[t1.value].top().doubleValue =
              (t2.type == PVariable ? rpnNames[t2.value].top().doubleValue
                                    : t2.doubleValue);
        } else if (rpnNames[t1.value].top().type == TypeString) {
          newT.type = PStringValue;
          newT.stringValue = rpnNames[t1.value].top().stringValue =
              (t2.type == PVariable ? rpnNames[t2.value].top().stringValue
                                    : t2.stringValue);
        } else if (rpnNames[t1.value].top().type == TypeBool) {
          newT.type = PBoolValue;
          newT.boolValue = rpnNames[t1.value].top().boolValue =
              (t2.type == PVariable ? rpnNames[t2.value].top().boolValue
                                    : t2.boolValue);
        }
      } else if (tkn.value == "+") {
        if ((t1.type == PIntValue) ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          newT.type = PIntValue;
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.intValue = v1 + v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          newT.type = PDoubleValue;
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.doubleValue = v1 + v2;
        } else if (t1.type == PStringValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeString)) {
          newT.type = PStringValue;
          string v1 =
              (t1.type == PStringValue ? t1.stringValue
                                       : rpnNames[t1.value].top().stringValue);
          string v2 =
              (t2.type == PStringValue ? t2.stringValue
                                       : rpnNames[t2.value].top().stringValue);
          newT.stringValue = v1 + v2;
        }
      } else if (tkn.value == "-") {
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          newT.type = PIntValue;
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.intValue = v1 - v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          newT.type = PDoubleValue;
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.doubleValue = v1 - v2;
        }
      } else if (tkn.value == "**") {
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          newT.type = PIntValue;
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.intValue = peng_pow(v1, v2);
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          newT.type = PDoubleValue;
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.doubleValue = peng_pow(v1, v2);
        }
      } else if (tkn.value == "and") {
        newT.type = PBoolValue;
        bool v1 = (t1.type == PBoolValue ? t1.boolValue
                                         : rpnNames[t1.value].top().boolValue);
        bool v2 = (t2.type == PBoolValue ? t2.boolValue
                                         : rpnNames[t2.value].top().boolValue);
        newT.boolValue = v1 && v2;
      } else if (tkn.value == "xor") {
        newT.type = PBoolValue;
        bool v1 = (t1.type == PBoolValue ? t1.boolValue
                                         : rpnNames[t1.value].top().boolValue);
        bool v2 = (t2.type == PBoolValue ? t2.boolValue
                                         : rpnNames[t2.value].top().boolValue);
        newT.boolValue = v1 != v2;
      } else if (tkn.value == "or") {
        newT.type = PBoolValue;
        bool v1 = (t1.type == PBoolValue ? t1.boolValue
                                         : rpnNames[t1.value].top().boolValue);
        bool v2 = (t2.type == PBoolValue ? t2.boolValue
                                         : rpnNames[t2.value].top().boolValue);
        newT.boolValue = v1 || v2;
      } else if (tkn.value == "%") {
        newT.type = PIntValue;
        int v1 = (t1.type == PIntValue ? t1.intValue
                                       : rpnNames[t1.value].top().intValue);
        int v2 = (t2.type == PIntValue ? t2.intValue
                                       : rpnNames[t2.value].top().intValue);
        newT.intValue = v1 % v2;
      } else if (tkn.value == "/") {
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          newT.type = PIntValue;
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          if (v2 == 0)
            throw err("Division by zero");
          newT.intValue = v1 / v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          newT.type = PDoubleValue;
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          if (v2 == 0)
            throw err("Division by zero");
          newT.doubleValue = v1 / v2;
        }
      } else if (tkn.value == "*") {
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          newT.type = PIntValue;
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.intValue = v1 * v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          newT.type = PDoubleValue;
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.doubleValue = v1 * v2;
        }
      } else if (tkn.value == "<") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 < v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 < v2;
        }
      } else if (tkn.value == ">") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 > v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 > v2;
        }
      } else if (tkn.value == ">=") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 >= v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 >= v2;
        }
      } else if (tkn.value == "<=") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 <= v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 <= v2;
        }
      } else if (tkn.value == "==") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 == v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 == v2;
        } else if (t1.type == PStringValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeString)) {
          string v1 =
              (t1.type == PStringValue ? t1.stringValue
                                       : rpnNames[t1.value].top().stringValue);
          string v2 =
              (t2.type == PStringValue ? t2.stringValue
                                       : rpnNames[t2.value].top().stringValue);
          newT.boolValue = v1 == v2;
        } else if (t1.type == PBoolValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeBool)) {
          bool v1 =
              (t1.type == PBoolValue ? t1.boolValue
                                     : rpnNames[t1.value].top().boolValue);
          bool v2 =
              (t2.type == PBoolValue ? t2.boolValue
                                     : rpnNames[t2.value].top().boolValue);
          newT.boolValue = v1 == v2;
        }
      } else if (tkn.value == "!=") {
        newT.type = PBoolValue;
        if (t1.type == PIntValue ||
            (t1.type == PVariable &&
             rpnNames[t1.value].top().type == TypeInt)) {
          int v1 = (t1.type == PIntValue ? t1.intValue
                                         : rpnNames[t1.value].top().intValue);
          int v2 = (t2.type == PIntValue ? t2.intValue
                                         : rpnNames[t2.value].top().intValue);
          newT.boolValue = v1 != v2;
        } else if (t1.type == PDoubleValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeDouble)) {
          double v1 =
              (t1.type == PDoubleValue ? t1.doubleValue
                                       : rpnNames[t1.value].top().doubleValue);
          double v2 =
              (t2.type == PDoubleValue ? t2.doubleValue
                                       : rpnNames[t2.value].top().doubleValue);
          newT.boolValue = v1 != v2;
        } else if (t1.type == PStringValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeString)) {
          string v1 =
              (t1.type == PStringValue ? t1.stringValue
                                       : rpnNames[t1.value].top().stringValue);
          string v2 =
              (t2.type == PStringValue ? t2.stringValue
                                       : rpnNames[t2.value].top().stringValue);
          newT.boolValue = v1 != v2;
        } else if (t1.type == PBoolValue ||
                   (t1.type == PVariable &&
                    rpnNames[t1.value].top().type == TypeBool)) {
          bool v1 =
              (t1.type == PBoolValue ? t1.boolValue
                                     : rpnNames[t1.value].top().boolValue);
          bool v2 =
              (t2.type == PBoolValue ? t2.boolValue
                                     : rpnNames[t2.value].top().boolValue);
          newT.boolValue = v1 != v2;
        }
      }
      s.push(newT);
    } else if (tkn.type == PFunction) {
      if (debug)
        cout << "PFunction\n";
      const auto nestedIt = rpnMap.find(tkn.value);
      if (nestedIt == rpnMap.end()) {
        throw runtime_error("Function '" + tkn.value + "' is not declared");
      }
      const auto &nestedFunction = nestedIt->second;
      const size_t argsCnt = nestedFunction.first.size();
      vector<PToken> newArgs;
      newArgs.reserve(argsCnt);
      vector<PToken> tmpVec;
      tmpVec.reserve(argsCnt);
      for (size_t j = 0; j < argsCnt; ++j) {
        tmpVec.push_back(s.top());
        s.pop();
      }
      for (size_t j = argsCnt; j-- > 0;) {
        PToken oneArg = tmpVec[j];
        if (oneArg.type == PVariable) {
          PToken newOneArg = PToken();
          Variable curVariable = rpnNames[oneArg.value].top();
          int oneArgType = curVariable.type;
          if (oneArgType == TypeInt) {
            newOneArg.type = PIntValue;
            newOneArg.intValue = curVariable.intValue;
          } else if (oneArgType == TypeDouble) {
            newOneArg.type = PDoubleValue;
            newOneArg.doubleValue = curVariable.doubleValue;
          } else if (oneArgType == TypeString) {
            newOneArg.type = PStringValue;
            newOneArg.stringValue = curVariable.stringValue;
          } else if (oneArgType == TypeBool) {
            newOneArg.type = PBoolValue;
            newOneArg.boolValue = curVariable.boolValue;
          }
          newArgs.push_back(newOneArg);
        } else if (oneArg.type == PIntValue || oneArg.type == PDoubleValue ||
                   oneArg.type == PStringValue || oneArg.type == PBoolValue) {
          newArgs.push_back(oneArg);
        }
      }
      ++nestLvl;
      PToken result = exec(tkn.value, newArgs, nestLvl);
      --nestLvl;
      if (debug)
        cout << "Result type: " << result.type << "\n";
      if (result.type != PNull)
        s.push(result);
    } else if (tkn.type == PIO) {
      if (debug)
        cout << "PIO\n";
      if (tkn.value == "write") {
        if (debug)
          cout << "write\n";
        const size_t argsCnt = static_cast<size_t>(tkn.args.back());
        vector<PToken> tmpVec;
        tmpVec.reserve(argsCnt);
        for (size_t j = 0; j < argsCnt; ++j) {
          tmpVec.push_back(s.top());
          s.pop();
        }
        for (size_t j = argsCnt; j-- > 0;) {
          PToken t = tmpVec[j];
          if (t.type == PVariable) {
            Variable curArg = rpnNames[t.value].top();
            int curArgType = curArg.type;
            if (curArgType == TypeInt) {
              cout << curArg.intValue;
            } else if (curArgType == TypeDouble) {
              cout << curArg.doubleValue;
            } else if (curArgType == TypeString) {
              cout << curArg.stringValue;
            } else if (curArgType == TypeBool) {
              cout << (curArg.boolValue ? "true" : "false");
            }
          } else if (t.type == PIntValue) {
            cout << t.intValue;
          } else if (t.type == PDoubleValue) {
            cout << t.doubleValue;
          } else if (t.type == PStringValue) {
            cout << t.stringValue;
          } else if (t.type == PBoolValue) {
            cout << (t.boolValue ? "true" : "false");
          }
        }
        cout << "\n";
      } else if (tkn.value == "read") {
        if (debug)
          cout << "read\n";
        const size_t argsCnt = static_cast<size_t>(tkn.args.back());
        vector<PToken> tmpVec;
        tmpVec.reserve(argsCnt);
        for (size_t j = 0; j < argsCnt; ++j) {
          tmpVec.push_back(s.top());
          s.pop();
        }
        for (size_t j = argsCnt; j-- > 0;) {
          PToken t = tmpVec[j];
          if (t.type == PVariable) {
            int curVarType = rpnNames[t.value].top().type;
            if (curVarType == TypeInt) {
              cin >> rpnNames[t.value].top().intValue;
            } else if (curVarType == TypeDouble) {
              cin >> rpnNames[t.value].top().doubleValue;
            } else if (curVarType == TypeString) {
              cin >> rpnNames[t.value].top().stringValue;
            } else if (curVarType == TypeBool) {
              string tmpS;
              cin >> tmpS;
              rpnNames[t.value].top().boolValue = (tmpS == "true" ? 1 : 0);
            }
          }
        }
      }
    }
    ++i;
  }
  while (!rpnLastNames.empty() && rpnLastNames.top().second >= nestLvl) {
    rpnNames[rpnLastNames.top().first].pop();
    rpnLastNames.pop();
  }
  return {};
}
