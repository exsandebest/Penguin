#include <string>
#include <utility>

class TokenType {
public:
    int type, level;
    bool isFunction;
    std::vector<int> args;
    TokenType(int type, int level, bool isFunction = false)
            : type(type), level(level), isFunction(isFunction) {}
};

enum {  // Types
    TypeBool = 1,
    TypeString = 2,
    TypeInt = 3,
    TypeDouble = 4,
    TypeNull = 5,
    TypeSpecial = 6
};

enum {  // P - Poliz
    POperator = 1,
    PVariable = 2,
    PSpecial = 3,
    PType = 4,
    PFunction = 5,
    PUnaryOperation = 6,
    PBinaryOperation = 7,
    PIntValue = 8,
    PDoubleValue = 9,
    PStringValue = 10,
    PBoolValue = 11,
    PNull = 12,
    PIO = 13  // Input/output
};

class PToken {
public:
    int type;
    std::string value;
    std::vector<int> args;
    int intValue = 0;
    std::string stringValue;
    double doubleValue = 0.0;
    bool boolValue = false;
    PToken(int t, std::string s) : type(t), value(std::move(s)) {}
    PToken() : type(PNull) {}
};

class Variable {
public:
    int type;
    int intValue = 0;
    std::string stringValue;
    double doubleValue = 0.0;
    bool boolValue = false;
    explicit Variable(int t) : type(t){};
};

class Token {
public:
    int type;
    std::string value;
    int size;
    int line;
    bool isFunction;
    bool isSimpleVariable;
    Token(int type, const std::string &value, int line)
            : type(type),
              value(value),
              size(int(value.size())),
              line(line),
              isFunction(false),
              isSimpleVariable(false) {}
};

class expressionElement {
public:
    int type;
    bool isSimpleVariable;
    explicit expressionElement(int tp) : type(tp), isSimpleVariable(false) {}
};

enum {  // Token's types
    integerNumber = 1,
    doubleNumber = 2,
    stringConstant = 3,
    name = 4,
    variableType = 5,
    functionType = 6,
    sOperator = 7,
    logicalOperator = 8,
    assignmentOperator = 9,
    comparisonOperator = 10,
    binaryMathOperator = 11,
    unaryMathOperator = 12,
    semicolon = 13,
    openingBracket = 14,
    closingBracket = 15,
    openingBrace = 16,
    closingBrace = 17,
    comma = 18,
    readwriteOperator = 19,
    openingSquareBracket = 20,
    closingSquareBracket = 21,
//    importOperator = 22,
    logicalConstant = 23
};

enum {  // States
    inCycle = 1,
    inFor1 = 3,  // for (_________; i < n; ++i){}
    inFunction = 6
};
