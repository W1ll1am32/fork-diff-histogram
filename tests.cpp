#define CATCH_CONFIG_MAIN  /
#include "catch.hpp"       

#include "Tokenizer.h"
#include "Diff.h"
#include <memory>
#include <string>
#include <vector>

TEST_CASE("Character Tokenizer tests", "[tokenizer][character]") {
    auto tokenizer = CreateTokenizer(TokenizerMode::CHARACTER);
    
    SECTION("Basic encoding and decoding") {
        std::string text = "abc";
        auto tokens = tokenizer->Encode(text);
        
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
    
    SECTION("Unicode character handling") {
        std::string text = "привет";
        auto tokens = tokenizer->Encode(text);
        
        REQUIRE(tokens.size() == 6);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
    
    SECTION("Special characters") {
        std::string text = "a\nb\tc";
        auto tokens = tokenizer->Encode(text);
        
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
}

TEST_CASE("Tokenizer tests", "[tokenizer][word]") {
    auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
    
    SECTION("Basic word tokenization") {
        std::string text = "hello world";
        auto tokens = tokenizer->Encode(text);
        
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
    
    SECTION("Multiple spaces between words") {
        std::string text = "hello  world";
        auto tokens = tokenizer->Encode(text);
        
        REQUIRE(tokens.size() == 4);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
    
    SECTION("Punctuation handling") {
        std::string text = "hello, world!";
        auto tokens = tokenizer->Encode(text);

        REQUIRE(tokens.size() == 3);
        REQUIRE(tokenizer->Decode(tokens) == text);
    }
}

TEST_CASE("Diff tests", "[diff]") {
    SECTION("Identical texts") {
        std::string text1 = "This is a test";
        std::string text2 = "This is a test";
        
        auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
        Diff diff(std::move(tokenizer), text1, text2);
        
        REQUIRE(diff.AreTextsIdentical() == true);
    }
    
    SECTION("Simple insertion") {
        std::string text1 = "This is test";
        std::string text2 = "This is a test";
        
        auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
        Diff diff(std::move(tokenizer), text1, text2);
        
        REQUIRE(diff.AreTextsIdentical() == false);
    }
    
    SECTION("Simple deletion") {
        std::string text1 = "This is a test";
        std::string text2 = "This is test";
        
        auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
        Diff diff(std::move(tokenizer), text1, text2);
        
        REQUIRE(diff.AreTextsIdentical() == false);
    }
    
    SECTION("Simple substitution") {
        std::string text1 = "This is a test";
        std::string text2 = "This is the test";
        
        auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
        Diff diff(std::move(tokenizer), text1, text2);
        
        REQUIRE(diff.AreTextsIdentical() == false);
    }
    
    SECTION("Empty text handling") {
        std::string text1 = "";
        std::string text2 = "This is a test";
        
        auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
        Diff diff(std::move(tokenizer), text1, text2);
        
        REQUIRE(diff.AreTextsIdentical() == false);
    }

}

TEST_CASE("Diff format output tests", "[diff][format]") {
    std::string text1 = "line1\nline2\nline3\n";
    std::string text2 = "line1\nmodified line\nline3\n";
    
    auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
    Diff diff(std::move(tokenizer), text1, text2);
    
    SECTION("Unified format") {
        std::string unified_diff = diff.GetDiff(DiffFormat::HISTOGRAM);
        REQUIRE_FALSE(unified_diff.empty());
        REQUIRE(unified_diff.find("---") != std::string::npos);
        REQUIRE(unified_diff.find("+++") != std::string::npos);
        REQUIRE(unified_diff.find("@@") != std::string::npos);
    }
}

TEST_CASE("File comparison integration tests", "[integration]") {
    {
        std::ofstream file1("test_old.txt");
        file1 << "This is line 1.\nThis is line 2.\nThis is line 3.\n";
        file1.close();
        
        std::ofstream file2("test_new.txt");
        file2 << "This is line 1.\nThis is modified line.\nThis is line 3.\n";
        file2.close();
    }
    
    auto readFileToString = [](const std::string& fileName) -> std::string {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };
    
    std::string text1 = readFileToString("test_old.txt");
    std::string text2 = readFileToString("test_new.txt");
    
    REQUIRE_FALSE(text1.empty());
    REQUIRE_FALSE(text2.empty());
    
    auto tokenizer = CreateTokenizer(TokenizerMode::WORD);
    Diff diff(std::move(tokenizer), text1, text2);
    
    REQUIRE_FALSE(diff.AreTextsIdentical());
    
    std::string unified_diff = diff.GetDiff(DiffFormat::HISTOGRAM);
    REQUIRE(unified_diff.find("modified") != std::string::npos);
    
    std::remove("test_old.txt");
    std::remove("test_new.txt");
}
