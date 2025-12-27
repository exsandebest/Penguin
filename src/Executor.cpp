#include "Executor.h"
#include "SyntaxAnalyzer.h"

// Executes a given function with specified arguments and nesting level
PToken execute(const std::string &functionName, const std::vector<PToken> &args,
               int nestLvl) { // args contains ONLY VALUES (P...Value)
  return exec(functionName, args, nestLvl);
}
