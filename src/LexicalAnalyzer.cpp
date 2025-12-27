#include "LexicalAnalyzer.h"
#include "Main.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int line = 1;  // Tracks the current line number within the source file
std::string s; // Holds the entire source code content
std::vector<Token *>
    tokens; // Stores the sequence of tokens extracted from the source code

// Adds a token with specified type and value to the token vector
void addToken(int type, const std::string &value) {
  tokens.push_back(new Token(type, value, line));
}

// Resets lexical analyzer state for a fresh run (used by tests)
void resetLexicalAnalyzer() {
  for (auto *token : tokens) {
    delete token;
  }
  tokens.clear();
  s.clear();
  line = 1;
}

// Returns true if the character is a letter or digit, false otherwise
bool ld(char c) { return isdigit(c) || isalpha(c); }

// Checks if the substring from index i matches any reserved word, returns true
// if match found, false otherwise
bool detectReserved(const std::string &str, int i) {
  std::string ts;
  for (int j = i; j < i + str.length() && j < s.length(); ++j) {
    ts += s[j];
    if (s[j] != str[j - i])
      return false;
  }
  if (ts == str && !ld(s[i + str.length()])) {
    return true;
  } else {
    return false;
  }
}

// Parses words from index i, identifies reserved words, variable types, or
// names, and adds them as tokens Returns the index after the parsed word
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
          throw std::runtime_error("Incorrect number constant");
        }
      } else {
        addToken(doubleNumber, resValue);
        return i;
      }
    } else {
      throw std::runtime_error("Incorrect number constant");
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
      throw std::runtime_error("Incorrect number constant");
    }
  } else {
    addToken(integerNumber, resValue);
    return i;
  }
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

// Cycle function to parse tokens
// Handles various token types including operators, brackets, and constants
int parse() {
  int i = 0;
  std::string ts;
  while (i < s.length()) {
    if (s[i] == ';') {
      addToken(semicolon, ";");
      ++i;
    } else if ((i + 1 < s.length()) &&
               (ts + s[i] + s[i + 1] == "==" || ts + s[i] + s[i + 1] == "<=" ||
                ts + s[i] + s[i + 1] == ">=" || ts + s[i] + s[i + 1] == "!=")) {
      addToken(comparisonOperator, ts + s[i] + s[i + 1]);
      i += 2;
    } else if (s[i] == '=') {
      addToken(assignmentOperator, std::string(1, s[i]));
      ++i;
    } else if ((i + 1 < s.length()) && (ts + s[i] + s[i + 1] == "**")) {
      addToken(binaryMathOperator, ts + s[i] + s[i + 1]);
      i += 2;
    } else if (s[i] == '<' || s[i] == '>') {
      addToken(comparisonOperator, std::string(1, s[i]));
      ++i;
    } else if ((i + 1 < s.length()) &&
               (ts + s[i] + s[i + 1] == "++" || ts + s[i] + s[i + 1] == "--")) {
      addToken(unaryMathOperator, ts + s[i] + s[i + 1]);
      i += 2;
    } else if (s[i] == '/' || s[i] == '*' || s[i] == '%') {
      addToken(binaryMathOperator, std::string(1, s[i]));
      ++i;
    } else if (s[i] == '-') {
      if (tokens.empty()) {
        addToken(unaryMathOperator, "-");
      } else {
        Token *t = tokens.back();
        if (t->type == name || t->type == stringConstant ||
            t->type == doubleNumber || t->type == integerNumber ||
            t->type == logicalConstant || t->type == closingBracket) {
          addToken(binaryMathOperator, "-");
        } else {
          addToken(unaryMathOperator, "-");
        }
      }
      ++i;
    } else if (s[i] == '+') {
      if (tokens.empty()) {
        addToken(unaryMathOperator, "+");
      } else {
        Token *t = tokens.back();
        if (t->type == name || t->type == stringConstant ||
            t->type == doubleNumber || t->type == integerNumber ||
            t->type == logicalConstant || t->type == closingBracket) {
          addToken(binaryMathOperator, "+");
        } else {
          addToken(unaryMathOperator, "+");
        }
      }
      ++i;
    } else if (isdigit(s[i]) || s[i] == '.') {
      i = parseNumber(i);
    } else if (s[i] == '"') {
      i = parseString(i);
    } else if (isalpha(s[i])) {
      i = parseWord(i);
    } else if (s[i] == '(') {
      addToken(openingBracket, "(");
      ++i;
    } else if (s[i] == ')') {
      addToken(closingBracket, ")");
      ++i;
    } else if (s[i] == '{') {
      addToken(openingBrace, "{");
      ++i;
    } else if (s[i] == '}') {
      addToken(closingBrace, "}");
      ++i;
    } else if (s[i] == '[') {
      addToken(openingSquareBracket, "[");
      ++i;
    } else if (s[i] == ']') {
      addToken(closingSquareBracket, "]");
      ++i;
    } else if (s[i] == ',') {
      addToken(comma, ",");
      ++i;
    } else if (s[i] == '!') {
      addToken(logicalOperator, "!");
      ++i;
    } else if (s[i] == '\n') {
      ++line;
      ++i;
    } else if (s[i] == ' ' || s[i] == '\r' || s[i] == '\t') {
      ++i;
    } else {
      throw std::runtime_error("Incorrect symbol : '" + std::string(1, s[i]) +
                               "'\n");
    }
  }
  return 0;
}

// Removes comment blocks from the input string and returns the cleaned string
// Throws an exception if comment syntax is incorrect
std::string deleteComments(std::string &str) {
  std::string res;
  const size_t len = str.length();
  bool outsideComment = true;
  for (size_t i = 0; i < len; ++i) {
    if (i + 1 < len) {
      const char cur = str[i];
      const char next = str[i + 1];
      if (outsideComment && cur == '/' && next == '*') {
        outsideComment = false;
        ++i;
        continue;
      }
      if (!outsideComment && cur == '*' && next == '/') {
        outsideComment = true;
        ++i;
        continue;
      }
      if (outsideComment && cur == '*' && next == '/') {
        throw std::runtime_error("Incorrect comments");
      }
    }
    if (outsideComment) {
      res += str[i];
    } else if (str[i] == '\n') {
      res += '\n';
    }
  }
  if (!outsideComment)
    throw std::runtime_error("Incorrect comments");
  return res;
}

// Runs Lexical Analysis
std::vector<Token *> runLexicalAnalysis(std::string input) {
  resetLexicalAnalyzer();
  s = std::move(input);
  s = deleteComments(s);
  parse();
  return tokens;
}
