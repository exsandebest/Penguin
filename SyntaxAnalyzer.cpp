//this is a prototype
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "Tokens.h"

using namespace std;

vector <Token*> v;
Token * cur;
int automatState = 0;

enum {integerNumber = 1, doubleNumber = 2, stringConstant = 3,
      name = 4, variableType = 5, functionType = 6, sOperator = 7, 
      logicalOperator = 8, readwriteOperator = 19, importOperator = 22, 
      assignmentOperator = 9, comparsionOperator = 10, binaryMathOperator = 11,
      unaryMathOperator = 12, semicolon = 13, openingBracket = 14, 
      closingBracket = 15, openingBrace = 16, closingBrace = 17,
      openingSquareBracket = 20, closingSquareBracket = 21,
      comma = 18};


string err (Token * t);
int curPos = -1;

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
void expression();
void operator_if();
void operator_continue();
void operator_break();
void operand();
void operator_io_read();
void operator_io_write();


string err (Token * t){
    string s = "";
    s += "Unexpected token: (" + to_string(t->type) + ") " + t->value + " on line " + to_string(t->line) + "\n";
    return s;
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
        if (!remove(argv[1])) throw string ("Unable to delete file: " + string(argv[1]));
        nextToken();
        program();
    } catch (string err){
        cout << err;
        return 0;
    }
}

void globals(){

}

int nextToken(){
    ++curPos;
    if (curPos >= v.size()) return 0;
    cur = v[curPos];
    return 1;
}

void program(){
    globals();
    functions();
}

void functions(){
    do {
        function();
    } while (nextToken());
}

void function(){
    if (!(cur->type == variableType || cur->type == 6)) throw err(cur);
    nextToken();
    if (cur->type != name) throw err(cur);

    nextToken();
    if (cur->type != openingBracket) throw err(cur);
    nextToken();
    arguments();
    if (cur->type != closingBracket) throw err(cur);
    nextToken();
    if (cur->type != openingBrace) throw err(cur);
    nextToken();
    block();
    if (cur->type != closingBrace) throw err(cur);
}

void arguments(){
    while (cur->type != closingBracket){
        if (cur->type != variableType) throw err(cur);
        nextToken();
        if (cur->type != name) throw err(cur);
        nextToken();
        if (cur->type == closingBracket) {
            nextToken();
            return;
        }
        if (cur->type != comma) throw err(cur);
        nextToken();
    }
}


void block(){
    do {
        operation();
    } while (nextToken());
}

void operation(){
    if (cur->type == variableType || cur->type == 19 || cur->type == 7 || cur->type == 4){
        _operator();
    }
}


void _operator() {
    if (cur->type == name) {
        operator_assignment();
    } else if (cur->type == readwriteOperator) {
        operator_input_output();
    } else if (cur->type == sOperator){
        operator_main();
    } else if (cur->type == variableType){

    }
}

void operator_main(){
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
    if (cur->type != semicolon) throw err(cur);
    nextToken();
}

void operator_break() {
    if (cur->type != semicolon) throw err(cur);
    nextToken();
}


void operator_while() {
    if (cur->type != openingBracket) throw err(cur);
    nextToken();
    expression();
    if (cur->type != closingBracket) throw err(cur);
    nextToken();
    if (cur->type != openingBrace) throw err(cur);
    nextToken();
    block();
    if (cur->type != closingBrace) throw err(cur);
    nextToken();
}

void operator_for(){
    if (cur->type != openingBracket) throw err(cur);
    nextToken();
    // Parse something
    if (cur->type != semicolon) throw err(cur);
    nextToken();
    expression();
    if (cur->type != semicolon) throw err(cur);
    nextToken();
    // Parse something
    if (cur->type != closingBracket) throw err(cur);
    nextToken();
    if (cur->type != openingBrace) throw err(cur);
    nextToken();
    block();
    if (cur->type != closingBrace) throw err(cur);
    nextToken();
}

void operator_if () {
    if (cur->type != openingBracket) throw err(cur);
    nextToken();
    expression();
    if (cur->type != closingBracket) throw err(cur);
    nextToken(); 
    if (cur->type != openingBrace) throw err(cur);
    nextToken();
    block();
    if (cur->type != closingBrace) throw err(cur);
    nextToken();
    if (cur->value != "else") return;
    nextToken();
    if (cur->value == "if"){
        operator_if();
        nextToken();
    } else if (cur->value == "{"){
        nextToken();
        block();
        if (cur->type != closingBrace) throw err(cur);
        nextToken();
    }
}


void operator_return(){
    operand();
    if (cur->type != semicolon) throw err(cur);
    nextToken();
}


void operator_input_output(){
    if (cur->value == "read"){
        operator_io_read();   
    } else if (cur->value == "write") {
        operator_io_write();
    }
}



void operator_assignment(){

}


void expression() {
   
}

void operand() {

}

void operator_io_read() {

}

void operator_io_write() {

}

