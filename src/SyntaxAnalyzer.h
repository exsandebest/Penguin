#ifndef PENGUIN_SYNTAXANALYZER_H
#define PENGUIN_SYNTAXANALYZER_H

#include <string>
#include "Main.h"
#include <vector>

std::runtime_error err(const std::string& errString, bool showLine);
std::string tokenToString(Token* t);
std::runtime_error errType(int currentType, int expectedType, bool showLine);

void addState(int state);
void delState(int state);

int nextToken();
void preprocessing();
void preprocessingFunction();
void program();
void functions();
void function();
void globals();
void arguments(const std::string& functionName, bool check = true);
void block();
void operator_();
void operator_assignment(int varType, const std::string& varName);
void operator_input_output();
void operator_main();
void operator_while();
void operator_for();
void operator_return();
std::pair<int, std::vector<PToken>> expression();
int operator_if(bool isRepeat = false);
void operator_continue();
void operator_break();
void operator_io_read();
void operator_io_write();
int arguments_to_call(const std::string& functionName);
void operator_variable_declaration();
void debugRpn(const std::string& fun);
PToken exec(const std::string& functionName, std::vector<PToken> args, int nestLvl);
int runLexicalAnalysis(std::vector<Token *> tokens, bool debugFlag);

#endif //PENGUIN_SYNTAXANALYZER_H
