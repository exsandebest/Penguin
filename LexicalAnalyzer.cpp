#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "Main.h"

int line = 1;  // Tracks the current line number within the source file
std::string s;  // Holds the entire source code content
std::vector<Token *> v;  // Stores the sequence of tokens extracted from the source code

const std::string reservedWords[] = {
        "break",
        "continue",
        "if",
        "else",
        "return",
        "while",
        "for"
};
const std::string reservedVariableTypes[] = {
        "bool",
        "string",
        "int",
        "double"
};
const std::string reservedFunctionTypes[] = {"null"};
const std::string reservedOperators[] = {"and", "or", "xor"};
const std::string reservedFunctions[] = {"read", "write"};
// const std::string reservedSpecialWords[] = {"import"};
const std::string reservedLogicalWords[] = {"true", "false"};

std::string deleteComments(std::string &str);
bool detectReserved(const std::string &str, int i);
int parseWord(int i);
int parseNumber(int i);
int parseString(int i);
void addToken(int type, const std::string& value);
bool ld(char c);

// Adds a token with specified type and value to the token vector
void addToken(int type, const std::string& value) {
    v.push_back(new Token(type, value, line));
}

// Returns true if the character is a letter or digit, false otherwise
bool ld(char c) {
    return isdigit(c) || isalpha(c);
}

// Checks if the substring from index i matches any reserved word, returns true if match found, false otherwise
bool detectReserved(const std::string &str, int i) {
    std::string ts;
    for (int j = i; j < i + str.length() && j < s.length(); ++j) {
        ts += s[j];
        if (s[j] != str[j - i]) return false;
    }
    if (ts == str && !ld(s[i + str.length()])) {
        return true;
    } else {
        return false;
    }
}

// Parses words from index i, identifies reserved words, variable types, or names, and adds them as tokens
// Returns the index after the parsed word
int parseWord(int i) {
    for (const std::string &word : reservedWords) {
        if (detectReserved(word, i)) {
            addToken(sOperator, word);
            return int(i + word.length());
        }
    }
    for (const std::string &word : reservedVariableTypes) {
        if (detectReserved(word, i)) {
            addToken(variableType, word);
            return int(i + word.length());
        }
    }
    for (const std::string &word : reservedFunctionTypes) {
        if (detectReserved(word, i)) {
            addToken(functionType, word);
            return int(i + word.length());
        }
    }
    for (const std::string &word : reservedOperators) {
        if (detectReserved(word, i)) {
            addToken(logicalOperator, word);
            return int(i + word.length());
        }
    }
    for (const std::string &word : reservedFunctions) {
        if (detectReserved(word, i)) {
            addToken(readwriteOperator, word);
            return int(i + word.length());
        }
    }
    for (const std::string &word : reservedLogicalWords) {
        if (detectReserved(word, i)) {
            addToken(logicalConstant, word);
            return int(i + word.length());
        }
    }
    std::string ts;
    while (ld(s[i])) {
        ts += s[i];
        ++i;
    }
    addToken(name, ts);
    return i;
}

// Parses and validates number constants from index i, adds them as tokens
// Returns the index after the parsed number
int parseNumber(int i) {
    std::string resValue;
    while (isdigit(s[i])) {
        resValue += s[i];
        ++i;
    }
    if (s[i] == '.') {
        ++i;
        if (isdigit(s[i])) {
            resValue += ".";
            while (isdigit(s[i])) {
                resValue += s[i];
                ++i;
            }
            if (s[i] == 'e' || s[i] == 'E') {
                ++i;
                resValue += "e";
                if (s[i] == '-') {
                    resValue += "-";
                    ++i;
                }
                if (isdigit(s[i])) {
                    while (isdigit(s[i])) {
                        resValue += s[i];
                        ++i;
                    }
                    addToken(doubleNumber, resValue);
                    return i;
                } else {
                    throw std::string("Incorrect number constant");
                }
            } else {
                addToken(doubleNumber, resValue);
                return i;
            }
        } else {
            throw std::string("Incorrect number constant");
        }
    } else if (s[i] == 'e' || s[i] == 'E') {
        ++i;
        resValue += "e";
        if (s[i] == '-') {
            resValue += "-";
            ++i;
        }
        if (isdigit(s[i])) {
            while (isdigit(s[i])) {
                resValue += s[i];
                ++i;
            }
            addToken(doubleNumber, resValue);
            return i;
        } else {
            throw std::string("Incorrect number constant");
        }
    } else {
        addToken(integerNumber, resValue);
        return i;
    }
    return 0;
}

// Parses string constants from index i, adds them as tokens
// Returns the index after the parsed string
int parseString(int i) {
    std::string resValue;
    ++i;
    while (s[i] != '"') {
        resValue += s[i];
        ++i;
    }
    ++i;
    addToken(stringConstant, resValue);
    return i;
}

// Recursive function to parse tokens from index i
// Handles various token types including operators, brackets, and constants
// Returns -1 if end of string is reached
int parse(int i) {
    if (i > s.length() - 1) return -1;
    std::string ts;

    if (s[i] == ';') {
        addToken(semicolon, ";");
        ++i;
        return parse(i);
    }
    if (ts + s[i] + s[i + 1] == "==" || ts + s[i] + s[i + 1] == "<=" ||
        ts + s[i] + s[i + 1] == ">=" || ts + s[i] + s[i + 1] == "!=") {
        addToken(comparisonOperator, ts + s[i] + s[i + 1]);
        return parse(i + 2);
    }
    if (s[i] == '=') {
        addToken(assignmentOperator, std::string(1, s[i]));
        return parse(i + 1);
    }
    if (s[i] == '~') {
        addToken(unaryMathOperator, std::string(1, s[i]));
        return parse(i + 1);
    }
    if (ts + s[i] + s[i + 1] == ">>" || ts + s[i] + s[i + 1] == "<<" ||
        ts + s[i] + s[i + 1] == "**") {
        addToken(binaryMathOperator, ts + s[i] + s[i + 1]);
        return parse(i + 2);
    }
    if (s[i] == '<' || s[i] == '>') {
        addToken(comparisonOperator, std::string(1, s[i]));
        return parse(i + 1);
    }
    if (ts + s[i] + s[i + 1] == "++" || ts + s[i] + s[i + 1] == "--") {
        addToken(unaryMathOperator, ts + s[i] + s[i + 1]);
        return parse(i + 2);
    }
    if (s[i] == '/' || s[i] == '*' || s[i] == '%' || s[i] == '^' || s[i] == '|' ||
        s[i] == '&') {
        addToken(binaryMathOperator, std::string(1, s[i]));
        return parse(i + 1);
    }
    if (s[i] == '-') {
        if (v.empty()) {
            addToken(unaryMathOperator, "-");
        } else {
            Token *t = v.back();
            if (t->type == name || t->type == stringConstant ||
                t->type == doubleNumber || t->type == integerNumber ||
                t->type == logicalConstant || t->type == closingBracket) {
                addToken(binaryMathOperator, "-");
            } else {
                addToken(unaryMathOperator, "-");
            }
        }
        return parse(i + 1);
    }
    if (s[i] == '+') {
        if (v.empty()) {
            addToken(unaryMathOperator, "+");
        } else {
            Token *t = v.back();
            if (t->type == name || t->type == stringConstant ||
                t->type == doubleNumber || t->type == integerNumber ||
                t->type == logicalConstant || t->type == closingBracket) {
                addToken(binaryMathOperator, "+");
            } else {
                addToken(unaryMathOperator, "+");
            }
        }
        return parse(i + 1);
    }
    if (isdigit(s[i])) {
        i = parseNumber(i);
        return parse(i);
    }
    if (s[i] == '"') {
        i = parseString(i);
        return parse(i);
    }
    if (isalpha(s[i])) {
        i = parseWord(i);
        return parse(i);
    }
    if (s[i] == '(') {
        addToken(openingBracket, "(");
        return parse(i + 1);
    }
    if (s[i] == ')') {
        addToken(closingBracket, ")");
        return parse(i + 1);
    }
    if (s[i] == '{') {
        addToken(openingBrace, "{");
        return parse(i + 1);
    }
    if (s[i] == '}') {
        addToken(closingBrace, "}");
        return parse(i + 1);
    }
    if (s[i] == '[') {
        addToken(openingSquareBracket, "[");
        return parse(i + 1);
    }
    if (s[i] == ']') {
        addToken(closingSquareBracket, "]");
        return parse(i + 1);
    }
    if (s[i] == ',') {
        addToken(comma, ",");
        return parse(i + 1);
    }
    if (s[i] == '!') {
        addToken(logicalOperator, "!");
        return parse(i + 1);
    }
    if (s[i] == '\n') {
        ++line;
        return parse(i + 1);
    }
    if (s[i] == ' ' || s[i] == '\r' || s[i] == '\t') return parse(i + 1);

    throw std::string("Incorrect symbol : '" + std::string(1, s[i]) + "'\n");
}

// Removes comment blocks from the input string and returns the cleaned string
// Throws an exception if comment syntax is incorrect
std::string deleteComments(std::string &str) {
    std::string res, ts;
    int i = 0;
    bool flag = true;
    while (i < str.length()) {
        if (ts + str[i] + str[i + 1] == "*/" && flag) {
            throw std::string("Incorrect comments");
        }
        if (ts + str[i] + str[i + 1] == "/*" && flag) {
            flag = false;
            i += 2;
            continue;
        }
        if (ts + str[i] + str[i + 1] == "*/" && !flag) {
            flag = true;
            i += 2;
            continue;
        }
        if (flag) {
            res += str[i];
        } else {
            if (str[i] == '\n') {
                res += str[i];
            }
        }
        ++i;
    }
    if (!flag) throw std::string("Incorrect comments");
    return res;
}

int main(int argc, char const *argv[]) {
    bool fromFile = false;
    try {
        if (argc % 2 == 0) {
            throw std::string("Incorrect arguments");
        }
        for (int i = 1; i < argc; i += 2) {
            if (strcmp(argv[i], "-i") == 0) {
                fromFile = true;
                std::ifstream inputFile(argv[i + 1]);
                if (!inputFile) {
                    throw std::string("Incorrect input file name");
                } else {
                    s.assign((std::istreambuf_iterator<char>(inputFile)),
                             (std::istreambuf_iterator<char>()));
                }
            } else if (strcmp(argv[i], "-o") == 0) {
                if (!std::freopen(argv[i + 1], "w", stdout)) {
                    throw std::string("Something wrong with the output file");
                }
            } else {
                throw std::string("Incorrect arguments");
            }
        }
        if (!fromFile) {
            getline(std::cin, s);
        }
        s = deleteComments(s);
        parse(0);
    } catch (std::string err) {
        std::cout << err;
        return 0;
    }
    std::cout << v.size() << "\n";
    for (auto &token : v) {
        std::cout << token->line << "\n" << token->type << "\n" << token->size << "\n"
                  << token->value << "\n";
    }
    return 0;
}
