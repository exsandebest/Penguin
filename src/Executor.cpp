#include "Main.h"
#include <string>
#include <iostream>
#include <stack>
#include "SyntaxAnalyzer.h"
#include "Math.h"

using namespace std;

// Executes a given function with specified arguments and nesting level
PToken execute(const string &functionName, vector<PToken> args,
            int nestLvl) {  // args contains ONLY VALUES (P...Value)
    if (debug)
        cout << "exec: " << functionName << ", args.size = " << args.size() << "\n";
    int startLvl = nestLvl;
    for (int i = 0; i < args.size(); ++i) {
        rpnNames[rpnMap[functionName].first[i].second].emplace(rpnMap[functionName].first[i].first);
        rpnLastNames.emplace(rpnMap[functionName].first[i].second, nestLvl);
        if (args[i].type == PIntValue) {
            rpnNames[rpnMap[functionName].first[i].second].top().intValue =
                    args[i].intValue;
        } else if (args[i].type == PDoubleValue) {
            rpnNames[rpnMap[functionName].first[i].second].top().doubleValue =
                    args[i].doubleValue;
        } else if (args[i].type == PStringValue) {
            rpnNames[rpnMap[functionName].first[i].second].top().stringValue =
                    args[i].stringValue;
        } else if (args[i].type == PBoolValue) {
            rpnNames[rpnMap[functionName].first[i].second].top().boolValue =
                    args[i].boolValue;
        }
    }
    if (debug) cout << "Args: ok\n";
    if (debug) debugRpn(functionName);
    vector<PToken> curRpn = rpnMap[functionName].second;
    stack<PToken> s;
    int i = 0;
    while (i < curRpn.size()) {
        if (debug) cout << "CURRENT NESTLVL: " << nestLvl << "\n";
        PToken tkn = curRpn[i];
        if (tkn.type == PVariable || tkn.type == PIntValue ||
            tkn.type == PDoubleValue || tkn.type == PStringValue ||
            tkn.type == PBoolValue) {
            if (debug) cout << "POperand: " << tkn.type << "\n";
            s.push(tkn);
        } else if (tkn.type == PType) {
            if (debug) cout << "PType: " << tkn.value << "\n";
            PToken t = s.top();
            s.pop();
            rpnNames[t.value].emplace(stringToType(tkn.value));
            rpnLastNames.emplace(t.value, nestLvl);
        } else if (tkn.type == PSpecial) {
            if (tkn.value == "levelup") {
                ++nestLvl;
            } else if (tkn.value == "leveldown") {
                while (!rpnLastNames.empty() &&
                       rpnLastNames.top().second >= nestLvl) {
                    rpnNames[rpnLastNames.top().first].pop();
                    rpnLastNames.pop();
                }
                --nestLvl;
            }
        } else if (tkn.type == POperator) {
            if (debug) cout << "POperator: " << tkn.value << "\n";
            if (tkn.value == "goto") {
                i = tkn.args.back();
                continue;
            } else if (tkn.value == "if") {
                PToken t = s.top();
                s.pop();
                if (t.boolValue) {
                    ++i;
                } else {
                    i = tkn.args.back();
                }
                continue;
            } else if (tkn.value == "return") {
                if (tkn.args.back()) {
                    PToken t = s.top();
                    s.pop();
                    if (t.type == PVariable) {
                        Variable curVariable = rpnNames[t.value].top();
                        int varType = curVariable.type;
                        if (varType == TypeInt) {
                            t.type = PIntValue;
                            t.intValue = curVariable.intValue;
                        } else if (varType == TypeDouble) {
                            t.type = PDoubleValue;
                            t.doubleValue = curVariable.doubleValue;
                        } else if (varType == TypeString) {
                            t.type = PStringValue;
                            t.stringValue = curVariable.stringValue;
                        } else if (varType == TypeBool) {
                            t.type = PBoolValue;
                            t.boolValue = curVariable.boolValue;
                        }
                    }
                    while (!rpnLastNames.empty() &&
                           rpnLastNames.top().second >= startLvl) {
                        rpnNames[rpnLastNames.top().first].pop();
                        rpnLastNames.pop();
                    }
                    return t;
                } else {
                    while (!rpnLastNames.empty() &&
                           rpnLastNames.top().second >= startLvl) {
                        rpnNames[rpnLastNames.top().first].pop();
                        rpnLastNames.pop();
                    }
                    return {};
                }
            }
        } else if (tkn.type == PUnaryOperation) {
            if (debug) cout << "PUnaryOperation: " << tkn.value << "\n";
            PToken t = s.top();
            s.pop();
            PToken newT = PToken();
            if (tkn.value == "!") {
                newT.type = PBoolValue;
                if (t.type == PVariable) {
                    newT.boolValue = !rpnNames[t.value].top().boolValue;
                } else if (t.type == PBoolValue) {
                    newT.boolValue = !t.boolValue;
                }
                s.push(newT);
            } else if (tkn.value == "-") {
                if (t.type == PVariable) {
                    if (rpnNames[t.value].top().type == TypeDouble) {
                        newT.type = PDoubleValue;
                        newT.doubleValue = -rpnNames[t.value].top().doubleValue;
                    } else if (rpnNames[t.value].top().type == TypeInt) {
                        newT.type = PIntValue;
                        newT.intValue = -rpnNames[t.value].top().intValue;
                    }
                } else if (t.type == PDoubleValue) {
                    newT.type = PDoubleValue;
                    newT.doubleValue = -t.doubleValue;
                } else if (t.type == PIntValue) {
                    newT.type = PIntValue;
                    newT.intValue = -t.intValue;
                }
                s.push(newT);
            } else if (tkn.value == "++") {
                // PVariable
                if (rpnNames[t.value].top().type == TypeDouble) {
                    newT.type = PDoubleValue;
                    newT.doubleValue = rpnNames[t.value].top().doubleValue =
                            rpnNames[t.value].top().doubleValue + 1;
                } else if (rpnNames[t.value].top().type == TypeInt) {
                    newT.type = PIntValue;
                    newT.intValue = rpnNames[t.value].top().intValue =
                            rpnNames[t.value].top().intValue + 1;
                }
                s.push(newT);
            } else if (tkn.value == "--") {
                // PVariable
                if (rpnNames[t.value].top().type == TypeDouble) {
                    newT.type = PDoubleValue;
                    newT.doubleValue = rpnNames[t.value].top().doubleValue =
                            rpnNames[t.value].top().doubleValue - 1;
                } else if (rpnNames[t.value].top().type == TypeInt) {
                    newT.type = PIntValue;
                    newT.intValue = rpnNames[t.value].top().intValue =
                            rpnNames[t.value].top().intValue - 1;
                }
                s.push(newT);
            }
        } else if (tkn.type == PBinaryOperation) {
            if (debug) cout << "PBinaryOperation: " << tkn.value << "\n";
            PToken t2 = s.top();
            s.pop();
            PToken t1 = s.top();
            s.pop();
            PToken newT = PToken();
            if (tkn.value == "=") {
                // PVariable
                // P...Value or PVariable
                if (rpnNames[t1.value].top().type == TypeInt) {
                    newT.type = PIntValue;
                    newT.intValue = rpnNames[t1.value].top().intValue =
                            (t2.type == PVariable ? rpnNames[t2.value].top().intValue
                                                  : t2.intValue);
                } else if (rpnNames[t1.value].top().type == TypeDouble) {
                    newT.type = PDoubleValue;
                    newT.doubleValue = rpnNames[t1.value].top().doubleValue =
                            (t2.type == PVariable ? rpnNames[t2.value].top().doubleValue
                                                  : t2.doubleValue);
                } else if (rpnNames[t1.value].top().type == TypeString) {
                    newT.type = PStringValue;
                    newT.stringValue = rpnNames[t1.value].top().stringValue =
                            (t2.type == PVariable ? rpnNames[t2.value].top().stringValue
                                                  : t2.stringValue);
                } else if (rpnNames[t1.value].top().type == TypeBool) {
                    newT.type = PBoolValue;
                    newT.boolValue = rpnNames[t1.value].top().boolValue =
                            (t2.type == PVariable ? rpnNames[t2.value].top().boolValue
                                                  : t2.boolValue);
                }
            } else if (tkn.value == "+") {
                if ((t1.type == PIntValue) ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    newT.type = PIntValue;
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.intValue = v1 + v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    newT.type = PDoubleValue;
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.doubleValue = v1 + v2;
                } else if (t1.type == PStringValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeString)) {
                    newT.type = PStringValue;
                    string v1 = (t1.type == PStringValue
                                 ? t1.stringValue
                                 : rpnNames[t1.value].top().stringValue);
                    string v2 = (t2.type == PStringValue
                                 ? t2.stringValue
                                 : rpnNames[t2.value].top().stringValue);
                    newT.stringValue = v1 + v2;
                }
            } else if (tkn.value == "-") {
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    newT.type = PIntValue;
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.intValue = v1 - v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    newT.type = PDoubleValue;
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.doubleValue = v1 - v2;
                }
            } else if (tkn.value == "**") {
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    newT.type = PIntValue;
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.intValue = peng_pow(v1, v2);
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    newT.type = PDoubleValue;
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.doubleValue = peng_pow(v1, v2);
                }
            } else if (tkn.value == "and") {
                newT.type = PBoolValue;
                bool v1 =
                        (t1.type == PBoolValue ? t1.boolValue
                                               : rpnNames[t1.value].top().boolValue);
                bool v2 =
                        (t2.type == PBoolValue ? t2.boolValue
                                               : rpnNames[t2.value].top().boolValue);
                newT.boolValue = v1 && v2;
            } else if (tkn.value == "xor") {
                newT.type = PBoolValue;
                bool v1 =
                        (t1.type == PBoolValue ? t1.boolValue
                                               : rpnNames[t1.value].top().boolValue);
                bool v2 =
                        (t2.type == PBoolValue ? t2.boolValue
                                               : rpnNames[t2.value].top().boolValue);
                newT.boolValue = v1 != v2;
            } else if (tkn.value == "or") {
                newT.type = PBoolValue;
                bool v1 =
                        (t1.type == PBoolValue ? t1.boolValue
                                               : rpnNames[t1.value].top().boolValue);
                bool v2 =
                        (t2.type == PBoolValue ? t2.boolValue
                                               : rpnNames[t2.value].top().boolValue);
                newT.boolValue = v1 || v2;
            } else if (tkn.value == "%") {
                newT.type = PIntValue;
                int v1 = (t1.type == PIntValue ? t1.intValue
                                               : rpnNames[t1.value].top().intValue);
                int v2 = (t2.type == PIntValue ? t2.intValue
                                               : rpnNames[t2.value].top().intValue);
                newT.intValue = v1 % v2;
            } else if (tkn.value == "/") {
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    newT.type = PIntValue;
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    if (v2 == 0) throw runtime_error("Division by zero");
                    newT.intValue = v1 / v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    newT.type = PDoubleValue;
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    if (v2 == 0) throw runtime_error("Division by zero");
                    newT.doubleValue = v1 / v2;
                }
            } else if (tkn.value == "*") {
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    newT.type = PIntValue;
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.intValue = v1 * v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    newT.type = PDoubleValue;
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.doubleValue = v1 * v2;
                }
            } else if (tkn.value == "<") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 < v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 < v2;
                }
            } else if (tkn.value == ">") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 > v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 > v2;
                }
            } else if (tkn.value == ">=") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 >= v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 >= v2;
                }
            } else if (tkn.value == "<=") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 <= v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 <= v2;
                }
            } else if (tkn.value == "==") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 == v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 == v2;
                } else if (t1.type == PStringValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeString)) {
                    string v1 = (t1.type == PStringValue
                                 ? t1.stringValue
                                 : rpnNames[t1.value].top().stringValue);
                    string v2 = (t2.type == PStringValue
                                 ? t2.stringValue
                                 : rpnNames[t2.value].top().stringValue);
                    newT.boolValue = v1 == v2;
                } else if (t1.type == PBoolValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeBool)) {
                    bool v1 =
                            (t1.type == PBoolValue ? t1.boolValue
                                                   : rpnNames[t1.value].top().boolValue);
                    bool v2 =
                            (t2.type == PBoolValue ? t2.boolValue
                                                   : rpnNames[t2.value].top().boolValue);
                    newT.boolValue = v1 == v2;
                }
            } else if (tkn.value == "!=") {
                newT.type = PBoolValue;
                if (t1.type == PIntValue ||
                    (t1.type == PVariable &&
                     rpnNames[t1.value].top().type == TypeInt)) {
                    int v1 = (t1.type == PIntValue ? t1.intValue
                                                   : rpnNames[t1.value].top().intValue);
                    int v2 = (t2.type == PIntValue ? t2.intValue
                                                   : rpnNames[t2.value].top().intValue);
                    newT.boolValue = v1 != v2;
                } else if (t1.type == PDoubleValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeDouble)) {
                    double v1 = (t1.type == PDoubleValue
                                 ? t1.doubleValue
                                 : rpnNames[t1.value].top().doubleValue);
                    double v2 = (t2.type == PDoubleValue
                                 ? t2.doubleValue
                                 : rpnNames[t2.value].top().doubleValue);
                    newT.boolValue = v1 != v2;
                } else if (t1.type == PStringValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeString)) {
                    string v1 = (t1.type == PStringValue
                                 ? t1.stringValue
                                 : rpnNames[t1.value].top().stringValue);
                    string v2 = (t2.type == PStringValue
                                 ? t2.stringValue
                                 : rpnNames[t2.value].top().stringValue);
                    newT.boolValue = v1 != v2;
                } else if (t1.type == PBoolValue ||
                           (t1.type == PVariable &&
                            rpnNames[t1.value].top().type == TypeBool)) {
                    bool v1 =
                            (t1.type == PBoolValue ? t1.boolValue
                                                   : rpnNames[t1.value].top().boolValue);
                    bool v2 =
                            (t2.type == PBoolValue ? t2.boolValue
                                                   : rpnNames[t2.value].top().boolValue);
                    newT.boolValue = v1 != v2;
                }
            }
            s.push(newT);
        } else if (tkn.type == PFunction) {
            if (debug) cout << "PFunction\n";
            int argsCnt = int(rpnMap[tkn.value].first.size());
            vector<PToken> newArgs;
            vector<PToken> tmpVec;
            for (int j = 0; j < argsCnt; ++j) {
                PToken oneArg = s.top();
                tmpVec.push_back(oneArg);
                s.pop();
            }
            for (int j = argsCnt - 1; j >= 0; --j) {
                PToken oneArg = tmpVec[j];
                if (oneArg.type == PVariable) {
                    PToken newOneArg = PToken();
                    Variable curVariable = rpnNames[oneArg.value].top();
                    int oneArgType = curVariable.type;
                    if (oneArgType == TypeInt) {
                        newOneArg.type = PIntValue;
                        newOneArg.intValue = curVariable.intValue;
                    } else if (oneArgType == TypeDouble) {
                        newOneArg.type = PDoubleValue;
                        newOneArg.doubleValue = curVariable.doubleValue;
                    } else if (oneArgType == TypeString) {
                        newOneArg.type = PStringValue;
                        newOneArg.stringValue = curVariable.stringValue;
                    } else if (oneArgType == TypeBool) {
                        newOneArg.type = PBoolValue;
                        newOneArg.boolValue = curVariable.boolValue;
                    }
                    newArgs.push_back(newOneArg);
                } else if (oneArg.type == PIntValue || oneArg.type == PDoubleValue ||
                           oneArg.type == PStringValue || oneArg.type == PBoolValue) {
                    newArgs.push_back(oneArg);
                }
            }
            ++nestLvl;
            PToken result = exec(tkn.value, newArgs, nestLvl);
            --nestLvl;
            if (debug) cout << "Result type: " << result.type << "\n";
            if (result.type != PNull) s.push(result);
        } else if (tkn.type == PIO) {
            if (debug) cout << "PIO\n";
            if (tkn.value == "write") {
                if (debug) cout << "write\n";
                int argsCnt = tkn.args.back();
                vector<PToken> tmpVec;
                for (int j = 0; j < argsCnt; ++j) {
                    PToken t = s.top();
                    tmpVec.push_back(t);
                    s.pop();
                }
                for (int j = argsCnt - 1; j >= 0; --j) {
                    PToken t = tmpVec[j];
                    if (t.type == PVariable) {
                        Variable curArg = rpnNames[t.value].top();
                        int curArgType = curArg.type;
                        if (curArgType == TypeInt) {
                            cout << curArg.intValue;
                        } else if (curArgType == TypeDouble) {
                            cout << curArg.doubleValue;
                        } else if (curArgType == TypeString) {
                            cout << curArg.stringValue;
                        } else if (curArgType == TypeBool) {
                            cout << (curArg.boolValue ? "true" : "false");
                        }
                    } else if (t.type == PIntValue) {
                        cout << t.intValue;
                    } else if (t.type == PDoubleValue) {
                        cout << t.doubleValue;
                    } else if (t.type == PStringValue) {
                        cout << t.stringValue;
                    } else if (t.type == PBoolValue) {
                        cout << (t.boolValue ? "true" : "false");
                    }
                }
                cout << "\n";
            } else if (tkn.value == "read") {
                if (debug) cout << "read\n";
                int argsCnt = tkn.args.back();
                vector<PToken> tmpVec;
                for (int j = 0; j < argsCnt; ++j) {
                    PToken t = s.top();
                    tmpVec.push_back(t);
                    s.pop();
                }
                for (int j = argsCnt - 1; j >= 0; --j) {
                    PToken t = tmpVec[j];
                    if (t.type == PVariable) {
                        int curVarType = rpnNames[t.value].top().type;
                        if (curVarType == TypeInt) {
                            cin >> rpnNames[t.value].top().intValue;
                        } else if (curVarType == TypeDouble) {
                            cin >> rpnNames[t.value].top().doubleValue;
                        } else if (curVarType == TypeString) {
                            cin >> rpnNames[t.value].top().stringValue;
                        } else if (curVarType == TypeBool) {
                            string tmpS;
                            cin >> tmpS;
                            rpnNames[t.value].top().boolValue = (tmpS == "true" ? 1 : 0);
                        }
                    }
                }
            }
        }
        ++i;
    }
    while (!rpnLastNames.empty() && rpnLastNames.top().second >= nestLvl) {
        rpnNames[rpnLastNames.top().first].pop();
        rpnLastNames.pop();
    }
    return {};
}