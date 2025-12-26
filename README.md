# Penguin
Penguin programming language
## Getting started
**You have to compile all files from ``src/``**  

**You can run ``scripts/make.cmd`` (for Windows) or ``scripts/make.sh`` (for Linux) to do that or use ``CMakeLists.txt``**

## Usage
``Penguin path/to/program.peng``  
## Examples
See examples at [`examples/`](https://github.com/exsandebest/Penguin/tree/master/examples)

Usage:
  - ``Penguin examples/a/a.peng``  
  - ``Penguin examples/b/b.peng < examples/b/in/1.in``
## Tests
**You can run tests with ``scripts/tests.bat`` or ``scripts/tests.sh``**  
## Docs
* [Grammar (Google Docs)](https://docs.google.com/document/d/1y9UAdCVIHkVw3AbSU_anU4KZSvI54mrA7OSpKjKvKgw) / [Grammar (Repo docs)](https://github.com/exsandebest/Penguin/tree/master/docs/grammar.ebnf)
* [Types of tokens (Google Sheets)](https://docs.google.com/spreadsheets/d/1OBjwfQxot8h_A8aIbHIXjujbpoGofYAS0elkgWege0g) / [Types of tokens (Repo docs)](https://github.com/exsandebest/Penguin/tree/master/docs/token_types.md)
* [Priority of operations (Google Sheets)](https://docs.google.com/spreadsheets/d/13oeLWDSUfdM0VBEn_MVFWN4l8zxK8_NX0-rdQLC79vI) / [Priority of operations (Repo docs)](https://github.com/exsandebest/Penguin/tree/master/docs/priority_table.md)
## Description
* Variable types
    * `int`: `1`, `-5`, `0` - `int a = 7;`
    * `double`: `0.2`, `.4`, `.5e2`, `7.3E-5` - `doule b = 1.0e-3;`
    * `string`: `"Hello World!"` - `string c = "teststring";`
    * `bool`: `true`, `false` - `bool d = true;`
* Operators
    * Logical operators: `and`, `or`, `xor`
    * Comparison operators: `>`, `<`, `>=`, `<=`, `==`, `!=`
    * Mathematical operators: `+`, `-`, `*`, `/`, `%`, `**` (power), `++` (prefix), `--` (prefix)
    * Assignments operators: `=`
* Comments
    * `/* ... */`
* Functions
    * `bool isEven(int a) { return a % 2 == 0; }`
    * Function operators: `return`
    * All functions are in global scope by default
      ```c++
      null main() { write(isEven(10)); }
      bool isEven(int a) { return a % 2 == 0; }
      /* No need to place a function before its usage */
      ```
* Cycles
    * `for`: `for (int i = 0; i < 10; ++i) { ... }`
    * `while`: `while (i < 5) { ... }`
    * Cycle operators: `break`, `continue`
* Conditional operators:
    * `if (x > 5) { ... }`
    * `else if (x < 2) { ... }`
    * `else { ... }`
* Built-in functions
    * `read(a, b);`
    * `write(x, " + ", y, " = ", x + y);`
* Local scopes  
    [`exsamples/a/a.peng`](https://github.com/exsandebest/Penguin/tree/master/examples/a/a.peng):
    ```c++
    null main() {
        int a;
        read(a);
        if (a > 2) {
            string a = "str";
            write(a);
        } else {
            bool a = true;
            write(a);
        }
        write(a + 2);
    }
    ```
