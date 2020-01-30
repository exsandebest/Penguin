#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "Tokens.h"

using namespace std;

vector <Token*> v;
Token * cur;

string err (Token * t);
void program();
void function();

string err (Token * t){
    string s = "";
    s += "Unexpected token: (" + to_string(t->type) + ") " + t->value + "\n";
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
        cur = v[0];
        program();
    } catch (string err){
        cout << err;
        return 0;
    }
}

void program(){

}

void function(){

}
