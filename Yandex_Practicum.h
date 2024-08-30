#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>
#include <tuple>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

vector<int> ReadNumbers(int count) {
    vector<int> numbers;
    int num = 0;
    for (int i = 0; i < count; i++) {
        cin >> num;
        numbers.push_back(num);
    }

    cin >> ws;
    return numbers;
}

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int doc_id, 
        const string& document, 
        DocumentStatus status, 
        const vector<int>& ratings) {
        
        ++document_count_;
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        docs_rating[doc_id] = ComputeAverageRating(ratings);
        docs_status[doc_id] = status;
        for (const string& word : words) {
            word_to_document_freqs_[word][doc_id] += inv_word_count;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query, 
        DocumentStatus status = DocumentStatus::ACTUAL) const {
        const Query query = ParseQuery(raw_query);
        vector<Document> matched_documents = FindAllDocuments(query, status);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        tuple<vector<string>, DocumentStatus> matched_docs;
        get<DocumentStatus>(matched_docs) = docs_status.at(document_id);
        Query query = ParseQuery(raw_query);

        vector <string>& matched_words = get<0>(matched_docs);
        set <string> matched_words_temp;

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) && 
                word_to_document_freqs_.at(word).count(document_id)) {
                return matched_docs;
            }
        }

        set<string> plus_words(query.plus_words);
        
        for (const string& word : plus_words) {
            if (word_to_document_freqs_.count(word)
                && word_to_document_freqs_.at(word).count(document_id) ) {
                matched_words.push_back(word);
            }
        }

        return matched_docs;
    }

    int GetDocumentCount() {
        return document_count_;
    }

private:
    // fields
    int document_count_ = 0;
    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, int> docs_rating;
    map<int, DocumentStatus> docs_status;

    // structs
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    //methods
    vector<Document> FindAllDocuments(const Query& query, 
        DocumentStatus status = DocumentStatus::ACTUAL) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0 ) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [id, relevance] : document_to_relevance) {
            if (docs_status.at(id) == status) {
                matched_documents.push_back({ id, relevance, docs_rating.at(id) });
            }
        }
        return matched_documents;
    }
    
    static int ComputeAverageRating(const vector<int>& ratings) {
        static int static_rating;
        static_rating = 0;

        if (ratings.empty()) {
            return static_rating;
        }

        for (int i : ratings) {
            static_rating += i;
        }

        static_rating /= static_cast<int>(ratings.size());

        return static_rating;
    }

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(document_count_ * 1.0 / word_to_document_freqs_.at(word).size());
    }
};

SearchServer CreateSearchServer()
{
    const string stop_words_joined = ReadLine();
    const int document_count = ReadLineWithNumber();

    SearchServer searcher;
    searcher.SetStopWords(stop_words_joined);
    string doc_text;
    string raiting_text;

    for (int document_id = 0; document_id < document_count; ++document_id) {
        doc_text = ReadLine();
        int num_count;
        cin >> num_count;
        vector<int> ratings = ReadNumbers(num_count);
        searcher.AddDocument(document_id, doc_text, DocumentStatus::ACTUAL, ratings);
    }

    return searcher;
}