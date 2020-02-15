enum {
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
    importOperator = 22
};

enum {
    inCycle = 1,
    inIf = 2,
    inFor1 = 3, //for (_________; i < n; ++i){}
    inFor3 = 5, //for (int i = 0; i < n; ___){}
    inFunction = 6,
    inElse = 7
};

enum { //ET - Expression type
    ETBool = 1,
    ETString = 2,
    ETNumber = 3
};
