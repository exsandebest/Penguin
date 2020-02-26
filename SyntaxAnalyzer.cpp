#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include "Main.h"

using namespace std;

int nestingLevel = 0;
int currentFunctionType = -1;
int preprocessorNestingLevel = 0;

std::map<std::string, std::stack<TokenType > > names;
std::stack<pair<string, int>> lastNames;

vector <Token*> v;
Token * cur;
stack<int> stateStack;
multiset<int> stateSet;

int curPos = -1;

string err(string errString);
string ttos (Token * t);
string errType();

void addState(int state);
void delState(int state);

int nextToken();
void preproccesing();
void preproccesingFunction();
void program();
void functions();
void function();
void globals();
void arguments(string functionName);
void block();
void operation();
void _operator();
void operator_assignment(int varType);
void operator_input_output();
void operator_main();
void operator_while();
void operator_for();
void operator_return();
int expression();
void operator_if();
void operator_continue();
void operator_break();
void operator_io_read();
void operator_io_write();
void arguments_to_call(string functionName, bool special);
void operator_variable_declaration();

string err (string errString = ""){
    if (errString == "") {
        string s = "";
        s += "Unexpected token: (" + to_string(cur->type) + ") '" + cur->value + "' on line " + to_string(cur->line) + "\n";
        return s;
    } else {
        return errString;
    }
}

string ttos (Token *) { //Token TO String
    string s = "";
    s += "Debug: (" + to_string(cur->type) + ") '" + cur->value + "' on line " + to_string(cur->line) + "\n";
    return s;
}

int stringToType(string s){
    if (s == "int") return TypeInt;
    if (s == "bool") return TypeBool;
    if (s == "string") return TypeString;
    if (s == "double") return TypeDouble;
    return TypeNull;
}

string errType(int currentType, int expectedType) {
    return string("Incorrect Expression Type: (" + to_string(currentType) + "), exprected (" + to_string(expectedType) + ")");
}


void addState(int state){
    stateStack.push(state);
    stateSet.insert(state);
}
void delState(int state){
    stateStack.pop();
    stateSet.erase(state);
}


int main (int argc, char const *argv[]){
    try {
        if (argc != 2) {
            throw string("Incorrect arguments");
        }
        ifstream fin(argv[1]);
        int n, line, type, size;
        string tmp;
        string value;
        getline(fin, tmp);
        n = stoi(tmp);
        for (int i = 0; i < n; ++i){
            getline(fin, tmp);
            line = stoi(tmp);
            getline(fin, tmp);
            type = stoi(tmp);
            getline(fin, tmp);
            size = stoi(tmp);
            getline(fin, value);
            value = value.substr(0,size);
            v.push_back(new Token(type, value, line));
        }
        fin.close();
        remove(argv[1]);
        nextToken();
        preprocessing();
        curPos = -1;
        nextToken();
        program();
        cout << "STATUS : OK";
    } catch (string err){
        cout << err;
        return 0;
    }
}


void preproccessing() {
    cout << "F: preproccesing\n";
    globals();
    do {
        preproccesingFunction();
    } while (nextToken());
}

void preproccesingFunction() {
    cout << "F: preproccesingFunction\n";
}


void globals(){
    cout << "F: globals\n";
    while (cur->value == "import"){
        nextToken();
        if (cur -> type != stringConstant) throw err();
        nextToken();
        if (cur->type != semicolon) throw err();
        nextToken();
    }
}

int nextToken(){
    ++curPos;
    if (curPos >= v.size()) return 0;
    cur = v[curPos];
    cout << ttos(cur);
    return 1;
}

void program(){
    cout << "F: program\n";
    globals();
    functions();
}

void functions(){
    cout << "F: functions\n";
    do {
        function();
    } while (nextToken());
}

void function(){
    cout << "F: function\n";
    if (!(cur->type == variableType || cur->type == functionType)) throw err();
    currentFunctionType = stringToType(cur->value);
    nextToken();
    if (cur->type != name) throw err();
    string curName = cur->value;
    if (!names[curName].empty()) throw err();
    names[curName].push(TokenType(currentFunctionType, nestingLevel, true));
    lastNames.push({curName, nestingLevel});
    nextToken();
    if (cur->type != openingBracket) throw err();
    nextToken();
    arguments(curName);
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != openingBrace) throw err();
    nextToken();
    addState(inFunction);
    block();
    delState(inFunction);
    if (cur->type != closingBrace) throw err();
    currentFunctionType = -1;
}

void arguments(string functionName){
    set<string> tmpSet;
    cout << "F: arguments\n";
    while (cur->type != closingBracket){
        if (cur->type != variableType) throw err();
        if (tmpSet.count(cur->value)) throw err("Duplicate name");
        names[functionName].top().args.push_back(stringToType(cur->value));
        tmpSet.insert(cur->value);
        nextToken();
        if (cur->type != name) throw err();
        nextToken();
        if (cur->type == closingBracket) {
            return;
        }
        if (cur->type != comma) throw err();
        nextToken();
    }
}


void block(){
    ++nestingLevel;
    cout << "F: block\n";
    do {
        operation();
    } while (cur->type != closingBrace);
    --nestingLevel;
    while (!lastNames.empty() && lastNames.top().second > nestingLevel){
        names[lastNames.top().first].pop();
        lastNames.pop();
    }
}

void operation(){
    cout << "F: operation\n";
    if (cur->type == variableType || cur->type == readwriteOperator || cur->type == sOperator || cur->type == name || cur->type == unaryMathOperator){
        _operator();
    } else {
        throw err();
    }
}


void _operator() {
    cout << "F: _operator\n";
    if (cur->type == name || cur->type == unaryMathOperator) {
        expression();
        if (cur->type != semicolon) throw err();
        nextToken();
    } else if (cur->type == readwriteOperator) {
        operator_input_output();
    } else if (cur->type == sOperator){
        operator_main();
    } else if (cur->type == variableType){
        operator_variable_declaration();
    } else {
        throw err();
    }
}

void operator_main(){
    cout << "F: operator_main\n";
    if (cur->value == "while"){
        nextToken();
        operator_while();
    } else if (cur->value == "for"){
        nextToken();
        operator_for();
    } else if (cur->value == "if"){
        nextToken();
        operator_if();
    } else if (cur->value == "return"){
        nextToken();
        operator_return();
    } else if (cur->value == "continue"){
        nextToken();
        operator_continue();
    } else if (cur->value == "break"){
        nextToken();
        operator_break();
    } else {
        throw err();
    }
}

void operator_continue(){
    cout << "F: operator_continue\n";
    if (stateSet.count(inCycle) == 0) throw err();
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_break() {
    cout << "F: operator_break\n";
    if (stateSet.count(inCycle) == 0) throw err();
    if (cur->type != semicolon) throw err();
    nextToken();
}


void operator_while() {
    cout << "F: operator_while\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    int curType = expression();
    if (curType != TypeBool) throw errType(curType, TypeBool);
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != openingBrace) throw err();
    nextToken();
    addState(inCycle);
    block();
    delState(inCycle);
    if (cur->type != closingBrace) throw err();
    nextToken();
}

void operator_for(){
    cout << "F: operator_for\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    addState(inFor1);
    if (cur->type == variableType) {
        operator_variable_declaration();
    } else {
        expression();
    }
    delState(inFor1);
    if (cur->type != semicolon) throw err();
    nextToken();
    int curType = expression();
    if (curType != TypeBool) throw errType(curType, TypeBool);
    if (cur->type != semicolon) throw err();
    nextToken();
    addState(inFor3);
    expression();
    delState(inFor3);
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != openingBrace) throw err();
    nextToken();
    addState(inCycle);
    block();
    delState(inCycle);
    if (cur->type != closingBrace) throw err();
    nextToken();
}

void operator_if () {
    cout << "F: operator_if\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    int curType = expression();
    if (curType != TypeBool) throw errType(curType, TypeBool);
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != openingBrace) throw err();
    nextToken();
    addState(inIf);
    block();
    delState(inIf);
    if (cur->type != closingBrace) throw err();
    nextToken();
    if (cur->value != "else") return;
    nextToken();
    if (cur->value == "if"){
        nextToken();
        operator_if();
        nextToken();
    } else if (cur->value == "{"){
        nextToken();
        addState(inElse);
        block();
        delState(inElse);
        if (cur->type != closingBrace) throw err();
        nextToken();
    }
}


void operator_return(){
    cout << "F: operator_return\n";
    if (stateSet.count(inFunction) == 0) throw err();
    int curType = expression();
    if (curType != currentFunctionType) throw errType(curType, currentFunctionType);
    if (cur->type != semicolon) throw err();
    nextToken();
}


void operator_input_output(){
    cout << "F: operator_input_output\n";
    if (cur->value == "read"){
        nextToken();
        operator_io_read();
    } else if (cur->value == "write") {
        nextToken();
        operator_io_write();
    }
}



void operator_assignment(int varType){
    cout << "F: operator_assignment\n";
    if (cur->type != assignmentOperator) throw err();
    nextToken();
    int curExpType = expression();
    if (curExpType != varType) throw errType(curExpType, varType);
    if (cur->type != semicolon) throw err();
    if (stateSet.count(inFor1) == 0) nextToken();
}

//max
int expression() {
    std::map<std::string, int> priority;
    std::stack<Token*> signs;
    std::vector<Token*> ans;

    int afterOpeningBracket = 0;
    bool mustBeObject = false;
    bool mustBeVar = false;
    int inFunction = 0;

    priority["."] = 2;
    priority["++"] = priority["!"] = 3;
    priority["*"] = priority["/"] = priority["%"] = 5;
    priority["+"] = priority["-"] = 6;
    priority["<"] = priority[">"] =
    priority["<="] =priority[">="] = 8;
    priority["=="] = priority["!="] = 1;
    priority["and"] = 13;
    priority["or"] = 14;
    priority["**"] = 4;
    priority["="] =priority["+="] =priority["-="] 
    =priority["*="] = priority["/="] =priority["%="] =15;
    
    while ((cur -> type != closingBracket || afterOpeningBracket > 0) 
            && cur -> type != semicolon && 
            (cur -> type != comma || afterOpeningBracket > 0)) {
        if (cur -> type == integerNumber || cur -> type == doubleNumber ||
            cur -> type == stringConstant || cur -> type == logicalConstant) {
            ans.push_back(cur);
            nextToken();
        } else if (cur -> type == name) {
            Token* q = cur;

            nextToken();
            if (cur -> type == openingBracket) {
                q -> isFunction = true;
                ++inFunction;
                ++afterOpeningBracket;

                signs.push(q);
                signs.push(cur);
                nextToken();
            } else {
                q -> isSimpleVariable = true;
                ans.push_back(q);
            }
        } else if (cur -> type == comma) {
            if (!inFunction)
                throw err();
            while (!signs.empty() && signs.top() -> type != openingBracket) {
                ans.push_back(signs.top());
                signs.pop();
            }
            if (signs.empty())
                throw err();
            nextToken();
        } else if ((cur -> type == unaryMathOperator) ||
            (cur -> type == logicalOperator && cur -> value == "!")) {
            signs.push(cur);
            nextToken();
        } else if (cur -> type == binaryMathOperator ||
                   cur -> type == comparsionOperator ||
                   cur -> type == assignmentOperator ||
                   cur -> type == logicalOperator) {
            if (cur -> value == "=" || cur -> value == "**") {
                while(!signs.empty() && ((signs.top() -> type == unaryMathOperator) ||
                      (signs.top() -> type == logicalOperator 
                      && signs.top() -> value == "!") ||
                      priority[signs.top() -> value] < priority[cur -> value])) {
                    ans.push_back(signs.top());
                    signs.pop();
                 }
            } else {
                while(!signs.empty() && ((signs.top() -> type == unaryMathOperator) ||
                      (signs.top() -> type == logicalOperator 
                      && signs.top() -> value == "!") ||
                      priority[signs.top() -> value] <= priority[cur -> value])) {
                    ans.push_back(signs.top());
                    signs.pop();
                 }
            }

            signs.push(cur);
            nextToken();
        } else if (cur -> type == openingBracket) {
            ++afterOpeningBracket;
            signs.push(cur);
            nextToken();
        } else if (cur -> type == closingBracket) {
            --afterOpeningBracket;
            while(!signs.empty() && signs.top() -> type != openingBracket) {
                ans.push_back(signs.top());
                signs.pop();
            }
            if (signs.empty())
                throw err();
            else {
                signs.pop();
                if (!signs.empty() && signs.top() -> type == name) {
                    ans.push_back(signs.top());
                    signs.pop();
                    --inFunction;
                }
            }
            nextToken();
        }
    }
    while (!signs.empty()) {
        if (signs.top() -> type == openingBracket)
            throw err();
        ans.push_back(signs.top());
        signs.pop();
    }
    stack<expressionElement*> exec;
    std::map<std::string, std::stack<TokenType> >::iterator ptr;
    int counter = 0;

    for (int i = (int)ans.size() - 1; i >= 0; --i) {
        if (ans[i] -> type == name) {
            if (ans[i] -> isFunction) {
                ans[i] -> isSimpleVariable = false;
                ptr = names.find(ans[i] -> value);
                if (ptr -> second.empty() && !ptr -> second.top().isFunction)
                    throw err("Function '" + ans[i] -> value + "' is not declared");
                counter = ptr -> second.top().args.size() - 1;
                while (counter >= 0 && !exec.empty()) {
                    if (exec.top() -> type != ptr -> second.top().args[counter])
                        throw err("Incorrect arguments of function");
                    delete exec.top();
                    exec.pop();
                    --counter; 
                }
                if (counter > -1)
                    throw err("Incorrect number of arguments of function");
                exec.push(new expressionElement(ptr -> second.top().type));
            } else {
                ptr = names.find(ans[i] -> value);
                if (ptr -> second.empty() || ptr -> second.top().isFunction)
                    throw err("Variable '" + ans[i] -> value + "' is not declared");
                exec.push(new expressionElement(ptr -> second.top().type));
                exec.top() -> isSimpleVariable = true;

            }
        } else if (ans[i] -> type == integerNumber) {
            exec.push(new expressionElement(3));
        } else if (ans[i] -> type == doubleNumber) {
            exec.push(new expressionElement(4));
        } else if (ans[i] -> type == stringConstant) {
            exec.push(new expressionElement(2));
        } else if (ans[i] -> type == logicalConstant) {
            exec.push(new expressionElement(1));
        } else if (ans[i] -> type == unaryMathOperator) {
            if (ans[i] -> value == "++" || ans[i] -> value == "--") {
                if (!exec.top() -> isSimpleVariable)
                    throw err("Can't increment/decrement a non-variable");
            }
            if (!(exec.top() -> type == TypeInt || exec.top() -> type == TypeDouble)) {
                throw err("Can't make arifmethic operations to a non-number");
            }
            exec.top() -> isSimpleVariable = false;

        } else if (ans[i] -> type == logicalOperator && ans[i] -> value == "!") {
            if (exec.top() -> type != TypeBool) {
                throw err("Can't make logic operations to a non-logic expression");
            }
            exec.top() -> isSimpleVariable = false;
        } else if (ans[i] -> type == logicalOperator) {
            expressionElement* sec, *fir;
            sec = exec.top();
            exec.pop();
            fir = exec.top();
            if (sec -> type != TypeBool || fir -> type != TypeBool) {
                throw err();//несоответствие типов в логическом выражении
            }
            exec.top() -> isSimpleVariable = false;
            delete sec;

        } else if (ans[i] -> type == binaryMathOperator) {
            expressionElement* sec, *fir;
            sec = exec.top();
            exec.pop();
            fir = exec.top();
            if (ans[i] -> value == "+") {
                if (!((sec -> type == TypeInt && fir -> type == TypeInt) ||
                  (sec -> type == TypeDouble && fir -> type == TypeDouble) ||
                  (sec -> type == TypeString && fir -> type == TypeString)))
                throw err();//несоответствие типов в математическом/строковом выражении
            } else if (!((sec -> type == TypeInt && fir -> type == TypeInt) ||
                  (sec -> type == TypeDouble && fir -> type == TypeDouble))) {
                throw err();//несоответствие типов в математическом выражении
            }
            exec.top() -> isSimpleVariable = false;
            delete sec;
        } else if (ans[i] -> type == comparsionOperator) {
            expressionElement* sec, *fir;
            sec = exec.top();
            exec.pop();
            fir = exec.top();
            if (!((sec -> type == TypeInt && fir -> type == TypeInt) ||
                  (sec -> type == TypeDouble && fir -> type == TypeDouble) ||
                  (sec -> type == TypeString && fir -> type == TypeString))) {
                throw err();//несоответствие типов в математическом/строковом выражении
            }
            delete sec;
            exec.top() -> isSimpleVariable = false;
            exec.top() -> type = TypeBool;
        } else if (ans[i] -> type == assignmentOperator) {
            expressionElement* sec, *fir;
            sec = exec.top();
            exec.pop();
            fir = exec.top();
            if (!(fir -> isSimpleVariable) || fir -> type != sec -> type)
                throw err();
            delete sec;
            fir -> isSimpleVariable = false;
        } else
              throw err();
    }
    if (exec.size() != 1)
        throw err();
    int q = exec.top() -> type;
    delete exec.top();

    return q;
}
//max



void arguments_to_call(string functionName, bool special = 0) {
    cout << "F: arguments_to_call\n";
    int k = 0;
    while (cur->type != closingBracket){
        if (special) {
            if (cur->type != name) throw err();
            if (!names[cur->value].empty()) throw err();
            if (names[cur->value].top().isFunction) throw err();
            nextToken();
        } else {
            int curType = expression();
        }
        if (cur->type == closingBracket) break;
        if (cur->type != comma) throw err();
        nextToken();
        ++k;
    }
}

void operator_io_read() {
    cout << "F: operator_io_read\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    arguments_to_call("read",1);
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_io_write() {
    cout << "F: operator_io_write\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    arguments_to_call("write");
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_variable_declaration() {
    cout << "F: operator_variable_declaration\n";
    if (cur->type != variableType) throw err();
    int curVarType = stringToType(cur->value);
    nextToken();
    if (cur->type != name) throw err();
    string curName = cur->value;
    nextToken();
    if (cur->type == assignmentOperator){
        operator_assignment(curVarType);
        if (names[curName].top().level == nestingLevel) throw err();
        names[curName].push(TokenType(curVarType, nestingLevel));
        lastNames.push({curName, nestingLevel});
    } else if (cur->type == semicolon){
        if (names[curName].top().level == nestingLevel) throw err();
        names[curName].push(TokenType(curVarType, nestingLevel));
        lastNames.push({curName, nestingLevel});
        if (stateSet.count(inFor1) == 0) nextToken();
    } else {
        throw err();
    }
}
