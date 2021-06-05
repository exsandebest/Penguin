#include <fstream>
#include <iostream>
#include <string>

#ifdef __unix__

    #define exe ""
    #define prefix "./"

#elif defined(_WIN32) || defined(WIN32)

    #define exe ".exe"
    #define prefix ""

#endif

int main(int argc, char const *argv[]) {
    try {
        if (argc != 2) {
            throw std::string("Incorrect arguments");
        }

        std::string fileName = argv[1];
        std::string tokensFile = "tokens.tmp";

        if (std::ifstream(fileName)) {
            std::string startLA = std::string(prefix) + "LexicalAnalyzer" +
                                  std::string(exe) + " -i " + fileName + " -o " +
                                  tokensFile;
            system(startLA.c_str());
            std::string startSA = std::string(prefix) + "SyntaxAnalyzer" +
                                  std::string(exe) + " " + tokensFile;
            system(startSA.c_str());
        } else {
            throw std::string("File does not exist\n");
        }
    } catch (std::string err) {
        std::cout << "Error: " << err << "\n";
    }
    return 0;
}
