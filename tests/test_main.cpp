#include "Executor.h"
#include "LexicalAnalyzer.h"
#include "Math.h"
#include "SyntaxAnalyzer.h"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename L, typename R>
void expectEqual(const L &actual, const R &expected,
                 const std::string &message) {
  if (actual != expected) {
    throw std::runtime_error(message);
  }
}

void expectThrow(const std::function<void()> &fn,
                 const std::string &message) {
  bool threw = false;
  try {
    fn();
  } catch (const std::runtime_error &) {
    threw = true;
  }
  if (!threw) {
    throw std::runtime_error(message);
  }
}

void testLexicalTokenization() {
  auto tokens = runLexicalAnalysis("int x = 5;\nwrite(x);");

  expectEqual(tokens.size(), static_cast<size_t>(10),
              "Unexpected token count for simple program");
  expectEqual(tokens[0]->type, variableType,
              "First token should be variable type");
  expectEqual(tokens[1]->type, name, "Second token should be variable name");
  expectEqual(tokens[5]->type, readwriteOperator,
              "write should be parsed as read/write operator");
  expectEqual(tokens[5]->line, 2, "write token should be on the second line");

  resetLexicalAnalyzer();
  resetSyntaxAnalyzerState();
}

void testLexicalCommentsPreserveLines() {
  std::string input = "int x = 1; /* comment\nstill comment */ int y = 2;";
  auto tokens = runLexicalAnalysis(input);

  expectEqual(tokens.size(), static_cast<size_t>(10),
              "Tokens with comments removed should still parse");
  expectEqual(tokens[5]->line, 2,
              "Second declaration should be detected on the second line");
  resetLexicalAnalyzer();
  resetSyntaxAnalyzerState();
}

void testLexicalRejectsBadNumbers() {
  expectThrow([]() { runLexicalAnalysis("int x = 1..2;"); },
              "Incorrect number constant should throw");
  resetLexicalAnalyzer();
  resetSyntaxAnalyzerState();
}

void testSyntaxRequiresMain() {
  const std::string program = "int foo() { return 1; }";
  auto tokens = runLexicalAnalysis(program);

  expectThrow(
      [&tokens]() { runLexicalAnalysis(tokens, false); },
      "Program without main function should fail during syntax analysis");

  resetSyntaxAnalyzerState();
  resetLexicalAnalyzer();
}

void testSyntaxChecksReturnType() {
  const std::string program =
      "bool isPositive(int value) { return 1; }\n"
      "null main() { return; }";
  auto tokens = runLexicalAnalysis(program);

  expectThrow(
      [&tokens]() { runLexicalAnalysis(tokens, false); },
      "Returning wrong type should trigger a syntax error");

  resetSyntaxAnalyzerState();
  resetLexicalAnalyzer();
}

void testExecutorEvaluatesFunctions() {
  const std::string program =
      "int add(int a, int b) { return a + b; }\n"
      "null main() { return; }";

  auto tokens = runLexicalAnalysis(program);
  runLexicalAnalysis(tokens, false);

  PToken left(PIntValue, "");
  left.intValue = 2;
  PToken right(PIntValue, "");
  right.intValue = 3;

  PToken result = execute("add", {left, right}, 0);
  expectEqual(result.type, PIntValue, "Result of add should be an int");
  expectEqual(result.intValue, 5, "add(2, 3) should evaluate to 5");

  resetSyntaxAnalyzerState();
  resetLexicalAnalyzer();
}

void testMathPowGuards() {
  expectThrow([]() { peng_pow(2, -1); },
              "Integer power should reject negative exponent");

  expectThrow([]() { peng_pow(-2.0, 0.5); },
              "Negative base with fractional power should be rejected");
}

} // namespace

int main() {
  int failures = 0;

  const std::vector<std::pair<std::string, std::function<void()>>> tests = {
      {"Lexical tokenization", testLexicalTokenization},
      {"Lexical comment handling", testLexicalCommentsPreserveLines},
      {"Lexical rejects bad numbers", testLexicalRejectsBadNumbers},
      {"Syntax requires main", testSyntaxRequiresMain},
      {"Syntax return type checking", testSyntaxChecksReturnType},
      {"Executor function evaluation", testExecutorEvaluatesFunctions},
      {"Math power guardrails", testMathPowGuards},
  };

  for (const auto &[name, fn] : tests) {
    try {
      fn();
      std::cout << "[PASS] " << name << "\n";
    } catch (const std::exception &ex) {
      ++failures;
      std::cerr << "[FAIL] " << name << ": " << ex.what() << "\n";
    }
  }

  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return 1;
  }

  std::cout << "All unit tests passed\n";
  return 0;
}
