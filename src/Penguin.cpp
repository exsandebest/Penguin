#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "Executor.h"

int main(int argc, char const *argv[]) {
    try {
        if (argc < 2 | argc > 3) {
            throw std::runtime_error("Incorrect arguments\nUsage: Penguin program.peng [--debug]\n");
        }

        bool debug = false;
        if (argc == 3 && strcmp(argv[2], "--debug") == 0) {
            debug = true;
        }

        std::string fileName = argv[1];
        std::string input;
        std::ifstream inputFile(fileName);
        if (!inputFile) {
            throw std::runtime_error("Incorrect input file name");
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

        int LAStatus = runLexicalAnalysis(tokens, debug);

        std::vector<PToken> tmp;
        std::string startFunction = "main";
        PToken result = execute(startFunction, tmp, 0);

        return 0;
    } catch (std::runtime_error& err) {
        std::cout << "Error: " << err.what() << "\n";
    }
    return 0;
}
