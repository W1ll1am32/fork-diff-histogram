#include "Tokenizer.h"
#include "Diff.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>

std::string readFileToString(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Can't open file " << fileName << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::string oldFileName = "old.txt";
    std::string newFileName = "new.txt";

    if (argc == 2) {
        oldFileName = argv[1];
        newFileName = argv[2];
    }
    else if (argc < 2) {
        std::cout << "Too few arguments" << std::endl;
        std::cout << "Using default files: " << oldFileName << ", " << newFileName << std::endl;
    }

    std::string text1 = readFileToString(oldFileName);
    std::string text2 = readFileToString(newFileName);

    // Examples
    //std::string text1 = "This is the first text to compare.\nIt contains several lines.\nSome will be changed.";
    //std::string text2 = "This is the second text to compare.\nIt contains several lines with changes.\nA new line has been added.\nAnd one more line.";

    if (text1.empty() || text2.empty()) {
        std::cerr << "One of the files is empty or does not exist" << std::endl;
        return 1;
    }

    // Create Tokenizer
    auto tokenizer = CreateTokenizer(TokenizerMode::WORD);

    Diff diff(std::move(tokenizer), text1, text2);

    if (diff.Identical()) {
        std::cout << "Texts are identical" << std::endl;
        return 0;
    }

    // Output in Unified format
    std::cout << "\nUnified diff format:" << std::endl;
    std::cout << diff.GetDiff(DiffFormat::HISTOGRAM) << std::endl;

    return 0;
}
