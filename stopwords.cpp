#include "stopwords.h"

using namespace std;

const unordered_set<string>& get_stopwords() {
    static const unordered_set<string> stopwords = {
        "a", "an", "and", "are", "as", "at",
        "be", "but", "by",
        "for", "if", "in", "into", "is", "it",
        "no", "not", "of", "on", "or",
        "such", "that", "the", "their", "then",
        "there", "these", "they", "this", "to",
        "was", "will", "with"
    };
    return stopwords;
}
