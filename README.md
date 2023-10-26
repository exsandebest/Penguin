# Penguin
Penguin programming language
## Getting started
**You have to compile all these files before using:**
```console
Penguin.cpp
LexicalAnalyzer.cpp
SyntaxAnalyzer.cpp
```
**You can run ``make.cmd`` (for Windows) or ``make.sh`` (for Linux) to do that**  
**After that you can use ``./Penguin path/to/program.peng`` to start**
## Examples
See examples at [`examples/`](https://github.com/exsandebest/Penguin/tree/master/examples)  
``./Penguin examples/a/a.peng``
## Tests
** Also you can run tests with ``tests.bat`` or ``tests.sh``**
## Documents about language
* [Grammar](https://docs.google.com/document/d/1y9UAdCVIHkVw3AbSU_anU4KZSvI54mrA7OSpKjKvKgw)  
* [Types of tokens](https://docs.google.com/spreadsheets/d/1OBjwfQxot8h_A8aIbHIXjujbpoGofYAS0elkgWege0g)  
* [Priority of operations](https://docs.google.com/spreadsheets/d/13oeLWDSUfdM0VBEn_MVFWN4l8zxK8_NX0-rdQLC79vI)  
## Description
* Variable types
    * `int`: `1`, `-5`, `0` - `int a = 7`
    * `double`: `0.2`, `.4`, `2.5e2`, `7.3E-5` - `doule b = 1.0e-3`
    * `string`: `"Hello World!"` - `string c = "teststring"`
    * `bool`: `true`, `false` - `bool d = true`
* Operators
    * Logical operators: `and`, `or`, `xor`
    * Comparison operators: `>`, `<`, `>=`, `<=`, `==`, `!=`
    * Mathematical operators: `+`, `-`, `*`, `/`, `%`, `**` (power), `++` (prefix), `--` (prefix)
    * Assignments operators: `=`, `+=`, `-=`, `/=`, `%=`
* Comments
    * `/* ... */`
* Functions
    * `bool isEven(int a) { return a % 2 == 0 }`
* Cycles
    * `for`: `for (int i = 0; i < 10; ++i) { ... }`
    * `while`: `while (i < 5) { ... }`
* Conditional operators:
    * `if (x > 5) { ... }`
    * `else if (x < 2) { ... }`
    * `else { ... }`
* Built-in functions
    * `read(a, b)`
    * `write(x, " + ", y, " = ", x + y)`
