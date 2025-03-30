#pragma once
#pragma once

#include <istream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <unordered_set>
#include <string_view>
#include <functional>

using TokenId = uint32_t;

enum class ParserMode { BYTES, UTF_8 };

enum class TokenizerMode {
    BPE,
    WORD,
    CHARACTER,
    WHITESPACE
};

class TokenInfo {
public:
    TokenInfo(TokenId id, const std::string& text);
    TokenInfo(TokenId id, std::string_view text);

    bool operator==(const TokenInfo& rhs) const;

    TokenId GetId() const;
    std::string_view GetText() const;

private:
    TokenId id_;
    std::string text_;
};

class Tokenizer {
public:
    Tokenizer(ParserMode parser_mode);
    virtual ~Tokenizer() = default;

    virtual std::vector<TokenId> Encode(const std::string& text) const = 0;
    virtual std::string Decode(const std::vector<TokenId>& tokens) const = 0;

    virtual const std::unordered_map<std::string, TokenId>& GetVocabulary() const = 0;

    virtual bool SaveVocabulary(const std::string& file_path) const = 0;
    virtual bool LoadVocabulary(const std::string& file_path) = 0;

protected:
    std::vector<std::string> SplitIntoUtf8Chars(const std::string& text) const;
    std::vector<std::string> SplitIntoWords(const std::string& text) const;

    bool IsUtf8Char(char c) const;

    ParserMode parser_mode_;
};

class BPETokenizer : public Tokenizer {
public:
    BPETokenizer(ParserMode parser_mode);

    std::vector<TokenId> Encode(const std::string& text) const override;
    std::string Decode(const std::vector<TokenId>& tokens) const override;

    const std::unordered_map<std::string, TokenId>& GetVocabulary() const override;

    bool SaveVocabulary(const std::string& file_path) const override;
    bool LoadVocabulary(const std::string& file_path) override;

    void Train(const std::vector<std::string>& corpus, int vocab_size, int min_frequency = 2);
    void AddMerges(const std::vector<std::pair<std::string, std::string>>& merges);

private:
    std::unordered_map<std::string, TokenId> vocab_;
    std::unordered_map<TokenId, std::string> inverse_vocab_;

    std::vector<std::pair<std::string, std::string>> merges_;

    std::vector<std::string> ApplyBPE(const std::string& word) const;
};

class CharacterTokenizer : public Tokenizer {
public:
    CharacterTokenizer(ParserMode parser_mode);

    std::vector<TokenId> Encode(const std::string& text) const override;
    std::string Decode(const std::vector<TokenId>& tokens) const override;

    const std::unordered_map<std::string, TokenId>& GetVocabulary() const override;

    bool SaveVocabulary(const std::string& file_path) const override;
    bool LoadVocabulary(const std::string& file_path) override;

private:
    std::unordered_map<std::string, TokenId> vocab_;
    std::unordered_map<TokenId, std::string> inverse_vocab_;
};

class WordTokenizer : public Tokenizer {
public:
    WordTokenizer(ParserMode parser_mode);

    std::vector<TokenId> Encode(const std::string& text) const override;
    std::string Decode(const std::vector<TokenId>& tokens) const override;

    const std::unordered_map<std::string, TokenId>& GetVocabulary() const override;

    bool SaveVocabulary(const std::string& file_path) const override;
    bool LoadVocabulary(const std::string& file_path) override;

private:
    std::unordered_map<std::string, TokenId> vocab_;
    std::unordered_map<TokenId, std::string> inverse_vocab_;
};

class WhitespaceTokenizer : public Tokenizer {
public:
    WhitespaceTokenizer(ParserMode parser_mode);

    std::vector<TokenId> Encode(const std::string& text) const override;
    std::string Decode(const std::vector<TokenId>& tokens) const override;

    const std::unordered_map<std::string, TokenId>& GetVocabulary() const override;

    bool SaveVocabulary(const std::string& file_path) const override;
    bool LoadVocabulary(const std::string& file_path) override;

private:
    std::unordered_map<std::string, TokenId> vocab_;
    std::unordered_map<TokenId, std::string> inverse_vocab_;
};

std::unique_ptr<Tokenizer> CreateTokenizer(
    TokenizerMode mode,
    ParserMode parser_mode = ParserMode::UTF_8
);
