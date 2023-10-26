#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"

int main(int argc, char const *argv[]) {
    try {
        if (argc < 2 | argc > 3) {
            throw std::string("Incorrect arguments\nUsage: Penguin program.peng [--debug]\n");
        }

        bool debug = false;
        if (argc == 3 && strcmp(argv[2], "--debug") == 0) {
            debug = true;
        }

        std::string fileName = argv[1];

        std::string input;

        std::ifstream inputFile(fileName);
        if (!inputFile) {
            throw std::string("Incorrect input file name");
        }
        input.assign((std::istreambuf_iterator<char>(inputFile)),
                 (std::istreambuf_iterator<char>()));

        std::vector<Token*> tokens = runLexicalAnalysis(input);

        if (debug) {
            std::cout << tokens.size() << "\n";
            std::cout << "LINE\t\t" << "TYPE\t\t" << "TOKEN\n";
            for (auto &token: tokens) {
                std::cout << token->line << "\t\t" << token->type << "\t\t"
                          << token->value << "\n";
            }
        }

        int status = runLexicalAnalysis(tokens, debug);

        return status;
    } catch (std::string err) {
        std::cout << "Error: " << err << "\n";
    }
    return 0;
}
