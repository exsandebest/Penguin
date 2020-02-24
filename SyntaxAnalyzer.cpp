#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <stack>
#include <set>
#include <map>
#include "Main.h"

using namespace std;

class tokenType {
    int nameType;
    std::vector<int> args;
    tokenType() {
        nameType = 0;
    }

};

std::map<std::string, std::stack<tokenType > > names;
std::stack<int> lastNames;

vector <Token*> v;
Token * cur;
stack<int> stateStack;
multiset<int> stateSet;

int curPos = -1;

string err();
string ttos (Token * t);
string errET();

void addState(int state);
void delState(int state);

int nextToken();
void program();
void functions();
void function();
void globals();
void arguments();
void block();
void operation();
void _operator();
void operator_assignment();
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
void arguments_to_call();
void operator_variable_declaration();

string err (){
    string s = "";
    s += "Unexpected token: (" + to_string(cur->type) + ") '" + cur->value + "' on line " + to_string(cur->line) + "\n";
    return s;
}

string ttos (Token *) { //Token TO String
    string s = "";
    s += "Debug: (" + to_string(cur->type) + ") '" + cur->value + "' on line " + to_string(cur->line) + "\n";
    return s;
}

string errET(int currentType, int exprectedType) {
    return string("Incorrect Expression Type: (" + to_string(currentType) + "), exprected (" + to_string(exprectedType) + ")");
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
        program();
        cout << "STATUS : OK";
    } catch (string err){
        cout << err;
        return 0;
    }
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
    if (!(cur->type == variableType || cur->type == 6)) throw err();
    nextToken();
    if (cur->type != name) throw err();
    nextToken();
    if (cur->type != openingBracket) throw err();
    nextToken();
    arguments();
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != openingBrace) throw err();
    nextToken();
    addState(inFunction);
    block();
    delState(inFunction);
    if (cur->type != closingBrace) throw err();
}

void arguments(){
    cout << "F: arguments\n";
    while (cur->type != closingBracket){
        if (cur->type != variableType) throw err();
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
    cout << "F: block\n";
    do {
        operation();
    } while (cur->type != closingBrace);
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
    }
}

void operator_continue(){
    cout << "F: operator_continue\n";
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_break() {
    cout << "F: operator_break\n";
    if (cur->type != semicolon) throw err();
    nextToken();
}


void operator_while() {
    cout << "F: operator_while\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    int curET = expression();
    //if (curET != ETBool) throw errET(curET, ETBool);
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
    int curET = expression();
    //if (curET != ETBool) throw errET(curET, ETBool);
    if (cur->type != semicolon) throw err();
    nextToken();
    expression();
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
    int curET = expression();
    //if (curET != ETBool) throw errET(curET, ETBool);
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
    expression();
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



void operator_assignment(){
    cout << "F: operator_assignment\n";
    if (cur->type != assignmentOperator) throw err();
    nextToken();
    expression();
    if (cur->type != semicolon) throw err();
    if (stateSet.count(inFor1) == 0) nextToken();
}

//max
int expression() {
    cout << "F: expression\n";

    enum {signExpect = 1, startExpect = 0, numExpect = 2,
          };
    bool canBeBeforeassign = true;

    int expressionState = startExpect;
   // std::queue<int> exp;

   while (cur -> type != closingBracket && cur -> type != semicolon && cur -> type != comma) {
       if (expressionState == 0) {
           if (cur -> type == openingBracket) {
                canBeBeforeassign = false;

                nextToken();
                expression();
                if (cur -> type != closingBracket)
                    throw err();
                nextToken();
                expressionState = 1;
           } else if (cur -> type == name) {
                 //check stack
                nextToken();

                if (cur -> type == openingBracket) {
                    nextToken();

                    while(cur -> type != closingBracket) {
                        expression();
                        if (cur -> type != comma &&
                            cur -> type != closingBracket)
                            throw err();
                        if (cur -> type != closingBracket)
                            nextToken();
                    }
                    nextToken();
                    canBeBeforeassign = false;
                }

                expressionState = 1;
           } else if (cur -> type == stringConstant) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
           } else if (cur -> type == integerNumber || cur -> type == doubleNumber) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
           } else if (cur -> type == logicalConstant) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
           } else if (cur -> type == unaryMathOperator) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 2;
               // if (expressionState == )
           } else if ((cur -> type == logicalOperator && cur -> value == "!")) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 2;
           } else
                throw err();
       } else if (expressionState == 1) {
            if (cur -> type == binaryMathOperator) {//math
                canBeBeforeassign = false;

                nextToken();
                expressionState = 0;
            } else if (cur -> type == comparsionOperator) {//logic
                canBeBeforeassign = false;

                nextToken();
                expressionState = 0;
            } else if (cur -> type == assignmentOperator &&
                canBeBeforeassign) {
                nextToken();
                expressionState = 0;
            } else if (cur -> type == logicalOperator) {//logic
                canBeBeforeassign = false;

                nextToken();
                expressionState = 0;
            }
            else
                throw err();
       } else if (expressionState == 2) {
            if (cur -> type == openingBracket) {
                canBeBeforeassign = false;

                nextToken();
                expression();
                if (cur -> type != closingBracket)
                    throw err();
                nextToken();
                expressionState = 1;
            } else if (cur -> type == name) {
                 //check stack

                nextToken();
                if (cur -> type == openingBracket) {
                    nextToken();

                    while(cur -> type != closingBracket) {
                        expression();
                        if (cur -> type != comma &&
                            cur -> type != closingBracket)
                            throw err();
                        if (cur -> type != closingBracket)
                            nextToken();
                    }
                    nextToken();
                    canBeBeforeassign = false;
                }
                expressionState = 1;
            } else if (cur -> type == stringConstant) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
            } else if (cur -> type == integerNumber || cur -> type == doubleNumber) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
            }  else if (cur -> type == logicalConstant) {
                canBeBeforeassign = false;

                nextToken();
                expressionState = 1;
            } else
                throw err();
       }

    }
    if (expressionState != 1)
        throw err();
    return 0;
}
//max


void arguments_to_call() {
    cout << "F: arguments_to_call\n";
    while (cur->type != closingBracket){
        expression();
        if (cur->type == closingBracket) break;
        if (cur->type != comma) throw err();
        nextToken();
    }
}

void operator_io_read() {
    cout << "F: operator_io_read\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    arguments_to_call();
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_io_write() {
    cout << "F: operator_io_write\n";
    if (cur->type != openingBracket) throw err();
    nextToken();
    expression();
    if (cur->type != closingBracket) throw err();
    nextToken();
    if (cur->type != semicolon) throw err();
    nextToken();
}

void operator_variable_declaration() {
    cout << "F: operator_variable_declaration\n";
    if (cur->type != variableType) throw err();
    nextToken();
    if (cur->type != name) throw err();
    nextToken();
    if (cur->type == assignmentOperator){
        operator_assignment();
    } else if (cur->type == semicolon){
        if (stateSet.count(inFor1) == 0) nextToken();
    } else {
        throw err();
    }
}
