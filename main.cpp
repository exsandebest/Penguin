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
            res->value = resValue;
            res->type = 2;
            v.push_back(res);
            return i;
        } else {
            throw "Incorrect number constant";
        }
    } else {
        res->value = resValue;
        res->type = 1;
        v.push_back(res);
        return i;
    }
}


int parseString(int i){
    string resValue = "";
    token * res = new token();
    ++i;
    while (s[i] != '"'){
        resValue += s[i];
        ++i;
    }
    ++i;
    res->type = 3;
    res->value = resValue;
    v.push_back(res);
    return i;
}

int parse(int i){

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
