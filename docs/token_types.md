| Type | Name                 | Description                                                                                   |
| ---- | -------------------- | --------------------------------------------------------------------------------------------- |
| 1    | integerNumber        | Integer literal without a sign                                                                |
| 2    | doubleNumber         | Floating-point literal with optional fraction and exponent                                    |
| 3    | stringConstant       | Double-quoted string literal (no escape processing)                                           |
| 4    | name                 | Identifier (letter/`_` followed by letters or digits)                                         |
| 5    | variableType         | Variable type keyword (`int`, `string`, `bool`, `double`)                                     |
| 6    | functionType         | Function return type keyword (`null`)                                                         |
| 7    | sOperator            | Structural keyword (`while`, `for`, `if`, `else`, `break`, `continue`, `return`)              |
| 8    | logicalOperator      | Logical operator (`and`, `or`, `xor`, `!`)                                                    |
| 9    | assignmentOperator   | Assignment operator (`=`)                                                                      |
| 10   | comparisonOperator   | Comparison operator (`==`, `<=`, `>=`, `<`, `>`, `!=`)                                        |
| 11   | binaryMathOperator   | Binary mathematical operator (`+`, `-`, `*`, `/`, `%`, `**`)                                   |
| 12   | unaryMathOperator    | Prefix arithmetic operator (`-`, `+`, `++`, `--`)                                              |
| 13   | semicolon            | `;`                                                                                            |
| 14   | openingBracket       | `(`                                                                                            |
| 15   | closingBracket       | `)`                                                                                            |
| 16   | openingBrace         | `{`                                                                                            |
| 17   | closingBrace         | `}`                                                                                            |
| 18   | comma                | `,`                                                                                            |
| 19   | readwriteOperator    | I/O keyword (`read`, `write`)                                                                  |
| 20   | openingSquareBracket | `[` (tokenized by the lexer, not used by the parser)                                          |
| 21   | closingSquareBracket | `]` (tokenized by the lexer, not used by the parser)                                          |
| 23   | logicalConstant      | Boolean literal (`true`, `false`)                                                              |

> Note: token type `22` is unused in the current implementation.
