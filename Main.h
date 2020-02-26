#include <string>

//class for semanthic analyse
class TokenType {
public:
    int type,
        level;
    bool isFunction;
    std::vector<int> args;
    TokenType(int type, int level, bool isFunction = 0): type(type), level(level), isFunction(isFunction) {

    }
};//endif

class Token {
public:
    int type;
    std :: string value;
    int size;
    int line;
    Token(int type, std :: string value, int line): type(type), value(value), size(value.size()), line(line) {}
};

enum { //Token's types
    integerNumber = 1,
    doubleNumber = 2,
    stringConstant = 3,
    name = 4,
    variableType = 5,
    functionType = 6,
    sOperator = 7,
    logicalOperator = 8,
    assignmentOperator = 9,
    comparsionOperator = 10,
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
    importOperator = 22,
    logicalConstant = 23
};

enum { //States
    inCycle = 1,
    inIf = 2,
    inFor1 = 3, //for (_________; i < n; ++i){}
    inFor3 = 5, //for (int i = 0; i < n; ___){}
    inFunction = 6,
    inElse = 7
};

enum { //Types
    TypeBool = 1,
    TypeString = 2,
    TypeInt = 3,
    TypeDouble = 4,
    TypeNull = 5
};

enum {
    isFunction = 1,
    isVariable = 0
};
