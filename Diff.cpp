#include "Diff.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <climits>

Diff::Diff(std::unique_ptr<Tokenizer> tokenizer,
    const std::string& text1,
    const std::string& text2,
    const std::string oldName,
    const std::string newName)
    : tokenizer_(std::move(tokenizer)), oldName_(oldName), newName_(newName) {

    from_tokens_ = tokenizer_->Encode(text1);
    /* Token Check
    for (const TokenId& token : from_tokens_) {
        std::cout << tokenizer_->Decode({ token }) << ",";
    }
    std::cout << std::endl;
    */
    to_tokens_ = tokenizer_->Encode(text2);
}

std::vector<TokenId> Diff::HistLCS(int from_left, int from_right, int to_left, int to_right) {
    // Skip equivalent items at top and bottom
    std::vector<TokenId> hs, ts;
    while (from_left < from_right && to_left < to_right && from_tokens_[from_left] == to_tokens_[to_left]) {
        hs.push_back(from_tokens_[from_left]);
        from_left++; to_left++;
    }
    while (from_left < from_right && to_left < to_right && from_tokens_[from_right - 1] == to_tokens_[to_right - 1]) {
        ts.push_back(from_tokens_[from_right - 1]);
        from_right--; to_right--;
    }
    reverse(ts.begin(), ts.end());

    // Build histogram
    struct Record {
        int from_count = 0, from_i = -1, to_count = 0, to_i = -1;
    };

    std::unordered_map<int, Record> hist;
    for (int i = from_left; i < from_right; i++) {
        hist[from_tokens_[i]].from_count++;
        hist[from_tokens_[i]].from_i = i;
    }
    for (int i = to_left; i < to_right; i++) {
        hist[to_tokens_[i]].to_count++;
        hist[to_tokens_[i]].to_i = i;
    }

    // Find lowest-occurrence item that appears in both
    int cmp = INT_MAX;
    int p = -1;
    for (const auto& [key, rec] : hist) {
        if (rec.from_count > 0 && rec.to_count > 0 && rec.from_count + rec.to_count < cmp) {
            p = key;
            cmp = rec.from_count + rec.to_count;
        }
    }

    if (p == -1) {
        hs.insert(hs.end(), ts.begin(), ts.end());
        return hs;
    }

    Record rec = hist[p];
    std::vector<TokenId> left = HistLCS(from_left, rec.from_i, to_left, rec.to_i);
    std::vector<TokenId> right = HistLCS(rec.from_i + 1, from_right, rec.to_i + 1, to_right);

    hs.insert(hs.end(), left.begin(), left.end());
    hs.push_back(p);
    hs.insert(hs.end(), right.begin(), right.end());
    hs.insert(hs.end(), ts.begin(), ts.end());

    return hs;
}

// Find the longest common subsequence (LCS)
std::vector<TokenId> Diff::LCS(DiffFormat format) {
    switch (format) {
        case DiffFormat::HISTOGRAM:
            return Diff::HistLCS(0, from_tokens_.size(), 0, to_tokens_.size());
        case DiffFormat::PATIENCE:
            return Diff::HistLCS(0, from_tokens_.size(), 0, to_tokens_.size()); // TO DO: Вставить реализацию Patience
        default:
            return Diff::HistLCS(0, from_tokens_.size(), 0, to_tokens_.size());
    }
}

void Diff::AddHunk(std::vector<Hunk>& hunks, int f_start, int f_end, int t_start, int t_end, bool has_prev, bool has_next) {
    Hunk hunk;
    int context_before = 0;
    int context_after = 0;

    if (has_prev && f_start > 0) {
        hunk.lines.push_back(" " + tokenizer_->Decode({ from_tokens_[f_start - 1] }));
        context_before = 1;
    }
    for (int f = f_start; f < f_end; f++) {
        hunk.lines.push_back("-" + tokenizer_->Decode({ from_tokens_[f] }));
    }
    for (int t = t_start; t < t_end; t++) {
        hunk.lines.push_back("+" + tokenizer_->Decode({ to_tokens_[t] }));
    }

    if (has_next && f_end < from_tokens_.size()) {
        hunk.lines.push_back(" " + tokenizer_->Decode({ from_tokens_[f_end] }));
        context_after = 1;
    }

    hunk.f_start = f_start - context_before + 1;
    hunk.t_start = t_start - context_before + 1;
    hunk.f_count = (f_end - f_start) + context_before + context_after;
    hunk.t_count = (t_end - t_start) + context_before + context_after;
    if (context_before > 0) {
        hunk.f_start -= context_before;
        hunk.t_start -= context_before;
    }
    hunks.push_back(hunk);
 };

std::string Diff::GetDiff(DiffFormat format) {
    std::vector<TokenId> lcs = Diff::LCS(format);
    /* LCS Check
    for (const TokenId& token : lcs) {
        std::cout << tokenizer_->Decode({ token }) << ",";
    }
    std::cout << std::endl;
    */
    std::vector<int> from_pos, to_pos;
    int i = 0, j = 0;
    for (const TokenId& token : lcs) {
        while (i < from_tokens_.size() && from_tokens_[i] != token) i++;
        if (i >= from_tokens_.size()) return "";
        from_pos.push_back(i);
        i++;
        while (j < to_tokens_.size() && to_tokens_[j] != token) j++;
        if (j >= to_tokens_.size()) return "";
        to_pos.push_back(j);
        j++;
    }

    std::vector<Hunk> hunks;
    int prev_f = 0, prev_t = 0;

    for (size_t k = 0; k <= lcs.size(); k++) {
        int curr_f = (k < from_pos.size()) ? from_pos[k] : from_tokens_.size();
        int curr_t = (k < to_pos.size()) ? to_pos[k] : to_tokens_.size();

        int f_start = prev_f;
        int f_end = curr_f;
        int t_start = prev_t;
        int t_end = curr_t;

        if (f_start < f_end || t_start < t_end) {
            bool has_prev = (k > 0);
            bool has_next = (k < from_pos.size());

            AddHunk(hunks, f_start, f_end, t_start, t_end, has_prev, has_next);
        }

        prev_f = curr_f + 1;
        prev_t = curr_t + 1;
    }

    std::stringstream diff;
    diff << "--- " << oldName_ << "\n";
    diff << "+++ " << newName_ << "\n";
    for (const Hunk& hunk : hunks) {
        diff << "@@ -" << hunk.f_start << "," << hunk.f_count << " +" << hunk.t_start << "," << hunk.t_count << " @@\n";
        for (const std::string& line : hunk.lines) {
            diff << line << "\n";
        }
    }

    return diff.str();
}

bool Diff::Identical() const {
    if (from_tokens_.size() != to_tokens_.size()) {
        return false;
    }
    for (size_t i = 0; i < from_tokens_.size(); ++i) {
        if (from_tokens_[i] != to_tokens_[i]) {
            return false;
        }
    }

    return true;
}

/*
// Function to compute the diff
void outputUnifiedDiff(const vector<pair<char, string>>& diff, const string& file1, const string& file2) {
    cout << "--- " << file1 << endl;
    cout << "+++ " << file2 << endl;

    int hunkStart = 1, count1 = 0, count2 = 0;
    vector<string> hunkLines;

    for (const auto& [type, token] : diff) {
        if (type == ' ') {
            if (!hunkLines.empty()) {
                cout << "@@ -" << hunkStart << "," << count1 << " +" << hunkStart << "," << count2 << " @@" << endl;
                for (const string& line : hunkLines) {
                    cout << line << endl;
                }
                hunkLines.clear();
            }
            hunkStart++;
        }
        else {
            hunkLines.push_back(string(1, type) + token);
            if (type == '-') count1++;
            if (type == '+') count2++;
        }
    }
    if (!hunkLines.empty()) {
        cout << "@@ -" << hunkStart << "," << count1 << " +" << hunkStart << "," << count2 << " @@" << endl;
        for (const string& line : hunkLines) {
            cout << line << endl;
        }
    }
}
*/