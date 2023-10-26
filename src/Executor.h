#ifndef PENGUIN_EXECUTOR_H
#define PENGUIN_EXECUTOR_H

#include "Main.h"
#include <string>
#include <vector>

PToken execute(const std::string &functionName, std::vector<PToken> args,
               int nestLvl);

#endif //PENGUIN_EXECUTOR_H
