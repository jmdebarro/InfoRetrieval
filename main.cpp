#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>
#include "stopwords.h"
using namespace std;

#define SOURCE_DIRECTORY "001"
#define EMPTY_FILE 0
#define TERM_FREQ_SCALING 1.2
#define EXIT_FUNC_FAILURE -1
#define DOC_LENGTH_NORMAL 0.75
#define PLACEHOLDER_VALUE 1

struct file_with_metadata {
/*  Struct containing file specific data  */
    string file_name;
    unsigned file_size;
    unordered_map<string, unsigned> word_map;

    bool operator==(const file_with_metadata& other) const {
        return file_size == other.file_size && file_name == other.file_name;
    }
};

struct doc_posting {
    string doc_id;
    float bm25;
};

bool doc_posting_sort(doc_posting a, doc_posting b) {
    return a.doc_id > b.doc_id;
}

struct file_struct_hash {
    size_t operator()(const file_with_metadata& file_object) const {
        return hash<string>()(file_object.file_name);
    }
};

string normalize(string s) {
    s.erase(
        remove_if(s.begin(), s.end(),
            [](unsigned char c) {
                return !isalnum(c);
            }),
        s.end()
    );

    transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) {
            return tolower(c);
        });

    return s;
}

void print_file_mapping(unordered_map<file_with_metadata, int, file_struct_hash> file_map, bool cond) {
    /* Prints a file_with_metadata and its word->count map */
    for (const auto& [file_key, file_value] : file_map) {
        cout << "File: " << file_key.file_name << " - Size: " << file_key.file_size;
        if (cond) { continue; }
        for (const auto& [word_key, word_value] : file_key.word_map) {
            cout << word_key << ": " << word_value << "\n";
        }
    }
}

float calc_tf_score(file_with_metadata file_object, string target_word, float avg_file_size) {
    /* 
        Calculates TF score for given document and word 
        https://www.geeksforgeeks.org/nlp/what-is-bm25-best-matching-25-algorithm/ 
    */
    unsigned file_size = file_object.file_size;
    unsigned word_frequency = 0;
    if (file_object.word_map.contains(target_word)) {
        word_frequency = file_object.word_map.contains(target_word);
    } else {
        return EXIT_FUNC_FAILURE;
    }

    float tf_id = 
    static_cast<float>(word_frequency) / 
        (word_frequency + TERM_FREQ_SCALING * 
        (1 - DOC_LENGTH_NORMAL + DOC_LENGTH_NORMAL * file_size / avg_file_size));
    
    return tf_id;
}

float calc_idf_score(unsigned file_appearances, string target_word, int file_count) {
    float idf_score = 
            log((file_count - file_appearances + 0.5) /
                     (file_appearances + 0.5));

    return idf_score;
}

int main() {
    /* 
        Iterate through all files, map key <word> : value <count>
        Find document : size
    */ 
    
    // Map that holds file name, file size, and word->word count map
    unordered_map<file_with_metadata, int, file_struct_hash> file_word_mapping;
    unordered_set<string> stopwords = get_stopwords();
    unsigned file_count = 0;
    unsigned total_word_count = 0;

    /*
        Iterate through each file, read it
        Documenting via map[file][word] = <count>
        This will be used to construct tf-idf
    */
    for (filesystem::directory_entry const& dir_entry : filesystem::directory_iterator(SOURCE_DIRECTORY)) {
        string cur_file = dir_entry.path().string();
        doc_posting cur_doc;

        file_count++;
        if (file_count % 100 == 0) {
            cout << "Processed: " << file_count << " files\n";
        }

        // Create variables for file_word_mapping struct above
        unordered_map<string, unsigned> file_map_init;
        file_with_metadata file_object = { cur_file, EMPTY_FILE, file_map_init};
        
        
        string cur_word;
        // Word count for each file in directory
        ifstream in(dir_entry.path());
        while (in >> cur_word) {
            // Sum every word for avg words per doc calc
            total_word_count++;

            // normalize all words and skip stopwords
            cur_word = normalize(cur_word);
            if (stopwords.contains(cur_word)) {
                continue;
            }
            
            file_object.file_size++;
            if (file_object.word_map.contains(cur_word)) {
                file_object.word_map[cur_word] += 1;
            } else {
                file_object.word_map[cur_word] = 1;
            }
        }
        file_word_mapping[file_object] = PLACEHOLDER_VALUE;
    }

    float avg_word_count = static_cast<float>(total_word_count) / file_count;
    // Map that has key: word, value: vector of documents with their associated bm25
    unordered_map<string, vector<doc_posting>> word_to_files_map;
    float tf_score = 0;
    float idf_score = 0;
    float bm25 = 0;
    /* Iterate through all files, calc score, and populate word_to_files_map */
    for (const auto& [file_object_ref, _] : file_word_mapping) {
        for (const auto& [word, count] : file_object_ref.word_map) {
            tf_score = calc_tf_score(file_object_ref, word, avg_word_count);
            idf_score = calc_idf_score(count, word, file_object_ref.file_size);
            bm25 = tf_score * idf_score;

            doc_posting cur_doc_posting = { file_object_ref.file_name, bm25 };
            word_to_files_map[word].push_back(cur_doc_posting);
        }
    }

    // print_file_mapping(file_word_mapping, true);
    return 0;
}