#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char const *argv[]){
    try{
        if (argc != 2) {
            throw string("Incorrect arguments");
        }
        string fileName = argv[1];
        string tokensFile = "tokens.tmp";
        if (ifstream(fileName)){
            string startLA = "LexicalAnalyzer.exe -i " + fileName + " -o " + tokensFile;
            system(startLA.c_str());
            string startSA = "SyntaxAnalyzer.exe " + tokensFile;
            system(startSA.c_str());
        } else {
            throw string ("File does not exist");
        }
    } catch (string err){
        cout << "Error: " << err;
        return 0;
    }
    return 0;
}
