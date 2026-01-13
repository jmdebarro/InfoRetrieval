#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>
#include "stopwords.h"

using namespace std;

#define SOURCE_DIRECTORY "001"
#define TERM_FREQ_SCALING 1.2f
#define DOC_LENGTH_NORMAL 0.75f

/* ===================== DATA STRUCTURES ===================== */

struct Document {
    uint32_t id;
    uint32_t length;
    unordered_map<string, unsigned> tf;
};

struct doc_posting {
    uint32_t doc_id;
    float bm25;
};

/* ===================== HELPERS ===================== */

string normalize(string s) {
    s.erase(remove_if(s.begin(), s.end(),
                      [](unsigned char c) {
                          return !isalnum(c);
                      }),
            s.end());

    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) {
                  return tolower(c);
              });

    return s;
}

/* ===================== MAIN ===================== */

int main() {
    unordered_set<string> stopwords = get_stopwords();

    vector<Document> documents;
    vector<string> doc_names;

    unordered_map<string, unsigned> doc_freq;   // DF per term

    uint32_t file_count = 0;
    uint64_t total_word_count = 0;

    documents.reserve(10000);
    doc_names.reserve(10000);
    doc_freq.reserve(100000);

    /* ===================== INDEX DOCUMENTS ===================== */

    for (const auto& dir_entry :
         filesystem::directory_iterator(SOURCE_DIRECTORY)) {

        Document doc;
        doc.id = file_count;
        doc.length = 0;

        unordered_set<string> seen_in_doc;

        ifstream in(dir_entry.path());
        string word;

        while (in >> word) {
            word = normalize(word);
            if (word.empty() || stopwords.contains(word))
                continue;

            doc.length++;
            total_word_count++;

            doc.tf[word]++;

            if (!seen_in_doc.contains(word)) {
                doc_freq[word]++;
                seen_in_doc.insert(word);
            }
        }

        documents.push_back(move(doc));
        doc_names.push_back(dir_entry.path().string());
        file_count++;

        if (file_count % 100 == 0) {
            cout << "Processed " << file_count << " documents\n";
        }
    }

    float avg_doc_length =
        static_cast<float>(total_word_count) / file_count;

    /* ===================== BUILD INVERTED INDEX ===================== */

    unordered_map<string, vector<doc_posting>> inverted_index;
    inverted_index.reserve(doc_freq.size());

    for (const auto& doc : documents) {
        for (const auto& [word, tf] : doc.tf) {

            float tf_score =
                tf / (tf + TERM_FREQ_SCALING *
                     (1.0f - DOC_LENGTH_NORMAL +
                      DOC_LENGTH_NORMAL *
                      doc.length / avg_doc_length));

            float idf =
                log((file_count - doc_freq[word] + 0.5f) /
                    (doc_freq[word] + 0.5f));

            inverted_index[word].push_back({
                doc.id,
                tf_score * idf
            });
        }
    }

    /* ===================== SORT POSTINGS ===================== */

    for (auto& [_, postings] : inverted_index) {
        sort(postings.begin(), postings.end(),
             [](const doc_posting& a,
                const doc_posting& b) {
                 return a.doc_id < b.doc_id;
             });
    }

    /* ===================== OUTPUT ===================== */

    for (const auto& [word, postings] : inverted_index) {
        cout << "Word: " << word << "\n";
        for (const auto& p : postings) {
            cout << "  " << doc_names[p.doc_id]
                 << "  " << p.bm25 << "\n";
        }
    }

    return 0;
}
