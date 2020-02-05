#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "Tokens.h"

using namespace std;

vector <Token*> v;
Token * cur;

string err (Token * t);
int curPos = -1;
int nextToken();
void program();
void functions();
void function();
void globals();
void arguments();
void block();

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

void nextToken(){
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
    while (nextToken()){
        function();
    }
}

void function(){
    if (!(cur->type == 5 || cur->type == 6)) err(cur);
    nextToken();
    if (cur->type != 4) err(cur);
    nextToken();
    if (cur->type != 14) err(cur);
    nextToken();
    arguments();
    if (cur->type != 15) err(cur);
    nextToken();
    if (cur->type != 16) err(cur);
    nextToken();
    block();
    if (cur->type != 17) err(cur);
}

void arguments(){
    while (cur->type != 15){
        if (cur->type != 5) err(cur);
        nextToken();
        if (cur->type != 4) err(cur);
        nextToken();
        if (cur->type == 15) {
            nextToken();
            return;
        }
        if (cur->type != 18) err(cur);
        nextToken();
    }
}
