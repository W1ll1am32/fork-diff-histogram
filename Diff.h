#pragma once

#include "Tokenizer.h"
#include <vector>
#include <utility>
#include <stdexcept>

enum class DiffFormat {
    HISTOGRAM,
    PATIENCE
};

struct EditLine {
    char type; // '-' for deletion, '+' for insertion, ' ' for common
    TokenId content;
    int original_line;
    int modified_line;
};

struct Hunk {
    int f_start, f_count;
    int t_start, t_count;
    std::vector<std::string> lines;
};

class Diff {
public:

    Diff(std::unique_ptr<Tokenizer> tokenizer,
        const std::string& text1,
        const std::string& text2);

    std::vector<TokenId> LCS(DiffFormat format);
    std::string GetDiff(DiffFormat format = DiffFormat::HISTOGRAM);

    bool Identical() const;

private:
    std::vector<TokenId> HistLCS(int from_left, int from_right, int to_left, int to_right);
    void AddHunk(std::vector<Hunk>& hunks, int f_start, int f_end, int t_start, int t_end, bool has_prev, bool has_next);

    std::unique_ptr<Tokenizer> tokenizer_;

    std::vector<TokenId> from_tokens_;
    std::vector<TokenId> to_tokens_;
};