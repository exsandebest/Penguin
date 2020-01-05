#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

struct token{
    int type;
    string value;
    token(){
        type = 0;
        value = "";
    }
};

string s;
vector <token*> v;
vector <string> reservedWords = {"break", "continue", "if", "else", "return", "while", "for"};
vector <string> reservedVariableTypes = {"bool", "string", "int", "double"};
vector <string> reservedFunctionTypes = {"null"};
vector <string> reservedOperators = {"and", "or", "xor"};

void addToken(int type, string value);
bool ld (char c);
bool detectReserved(string str, int i);
int parseWord(int i);
int parseNumber(int i);
int parseString(int i);

void addToken(int type, string value){
    token * res = new token();
    res->type = type;
    res->value = value;
    v.push_back(res);
}


bool ld (char c){
    return (isdigit(c) || isalpha(c));
}


bool detectReserved(string str, int i){
    string ts = "";
    for (int j = i; j < i + str.length(); ++j){
        ts += s[j];
    }
    if (ts == str && !ld(s[i+str.length()])) {
        return true;
    } else {
        return false;
    }
}

int parseWord(int i){
    for (string word : reservedWords) {
        if (detectReserved(word, i)) {
            addToken(6, word);
            return (i + word.length());
        }
    }
    for (string word : reservedVariableTypes) {
        if (detectReserved(word, i)) {
            addToken(5, word);
            return (i + word.length());
        }
    }
    for (string word : reservedFunctionTypes) {
        if (detectReserved(word, i)) {
            addToken(14, word);
            return (i + word.length());
        }
    }
    for (string word : reservedOperators) {
        if (detectReserved(word, i)) {
            addToken(7, word);
            return (i + word.length());
        }
    }
    string ts = "";
    while (ld(s[i])){
        ts += s[i];
        ++i;
    }
    addToken(4, ts);
    return i;
}



int parseNumber(int i){
    string resValue = "";
    token * res = new token();
    while (isdigit(s[i])){
        resValue += s[i];
        ++i;
    }
    if (s[i] == '.'){
        ++i;
        if (isdigit(s[i])){
            resValue += ".";
            while (isdigit(s[i])){
                resValue += s[i];
                ++i;
            }
            addToken(2,resValue);
            return i;
        } else {
            throw "Incorrect number constant";
        }
    } else {
        addToken(1,resValue);
        return i;
    }
}


int parseString(int i){
    string resValue = "";
    ++i;
    while (s[i] != '"'){
        resValue += s[i];
        ++i;
    }
    ++i;
    addToken(3,resValue);
    return i;
}

int parse(int i){
    if(s[i] == ';'){
        addToken(8,";");
        ++i;
        return parse(i);
    }
    if (s[i] == '+' || s[i] == '-' || s[i] == '/' || s[i] == '*' || s[i] == '%' || s[i] == '^'){
        addToken(13, s[i]);
        ++i;
        return parse(i);
    }
}

int main(){
    ifstream fin;
    fin.open("input.peng");
    ofstream fout;
    fout.open("output.txt");

    parse(0);

    for (int i = 0; i < v.size(); ++i){
        cout << "Number: " << i+1 << "\nType: " << v[i]->type << "\nValue: " << v[i]->value << "\n\n";
    }
    for (int i = 0; i < v.size(); ++i){
        fout << "Number: " << i+1 << "\nType: " << v[i]->type << "\nValue: " << v[i]->value << "\n\n";
    }
    return 0;
}
