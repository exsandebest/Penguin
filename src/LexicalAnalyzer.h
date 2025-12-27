#ifndef PENGUIN_LEXICALANALYZER_H
#define PENGUIN_LEXICALANALYZER_H

#include "Main.h"
#include <vector>

const std::string reservedWords[] = {"break",  "continue", "if", "else",
                                     "return", "while",    "for"};
const std::string reservedVariableTypes[] = {"bool", "string", "int", "double"};
const std::string reservedFunctionTypes[] = {"null"};
const std::string reservedOperators[] = {"and", "or", "xor"};
const std::string reservedFunctions[] = {"read", "write"};
const std::string reservedLogicalWords[] = {"true", "false"};

std::string deleteComments(std::string &str);
bool detectReserved(const std::string &str, int i);
int parseWord(int i);
int parseNumber(int i);
int parseString(int i);
void addToken(int type, const std::string &value);
void resetLexicalAnalyzer();
bool ld(char c);
std::vector<Token *> runLexicalAnalysis(std::string input);

#endif // PENGUIN_LEXICALANALYZER_H
