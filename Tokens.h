#include <string>

using namespace std;

class Token {
public:
    int type;
    string value;
    int size;
    int line;
    Token(int type, string value, int line): type(type), value(value), size(value.size()), line(line) {}
};
