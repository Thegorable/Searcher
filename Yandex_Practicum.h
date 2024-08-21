#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <functional>
#include <string>
#include <map>
#include <cmath>

#define DOUBLE static_cast<double>

using namespace std;

struct Document { int id; double relevance; };
const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result = 0;
	cin >> result;
	ReadLine();
	return result;
}

class SearchServer
{
private: // local structs define
	struct Query { set<string> plus_words; set<string> min_words; };

public:

	void AddDocument(int doc_id, const string& text) {
		const vector<string> words = SplitIntoWords(text, true);
		double doc_tf = 1.0 / DOUBLE(words.size());

		for (const string& word : words) {

			if (docs_content_.count(word)) {
				docs_content_[word][doc_id] += doc_tf;
			}
			else {
				docs_content_[word] = map<int, double>{ {doc_id, doc_tf} };
			}
		}

		docs_size_.push_back(words.size());
		docs_count_++;
	}

	Query ParseQuery(const string& text) const {
		Query query_words;

		for (const string& word : SplitIntoWords(text, true)) {

			if (word.at(0) == '-') {
				query_words.min_words.insert(word.substr(1));
			}
			else {
				query_words.plus_words.insert(word);
			}
		}

		return query_words;
	}

	void SetStopWords(const string& text) {
		if (text.size()) {

			for (const string& word : SplitIntoWords(text)) {
				stop_words.insert(word);
			}
		}
	}

	vector<Document> FindTopDocuments(const string& raw_query) const {
		const Query query_words = ParseQuery(raw_query);
		vector<Document> matched_docs = FindAllDocuments(query_words);

		sort(matched_docs.begin(), matched_docs.end(),
			[](const Document& doc1, const Document& doc2) -> bool {
				return doc1.relevance > doc2.relevance;
			});

		if (matched_docs.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_docs.resize(MAX_RESULT_DOCUMENT_COUNT);
		}

		return matched_docs;
	}

private: // methods

	double CalcIDF(const string& word) const
	{
		if (docs_content_.count(word))
		{
			return log(DOUBLE(docs_count_) / DOUBLE(docs_content_.at(word).size()));
		}
		else
		{
			return 0.0;
		}
	}

	vector<Document> FindAllDocuments(const Query& query_words) const
	{
		vector<Document> match_docs;
		map<int, int> match_ids;

		for (const string& word : query_words.plus_words) {
			double idf = CalcIDF(word);

			if (docs_content_.count(word)) {

				for (const auto& [id, tf] : docs_content_.at(word)) {

					if (!match_ids.count(id)) {
						match_ids[id] = match_docs.size();
						match_docs.push_back(Document{ id , idf * tf });
					}
					else {
						match_docs[match_ids.at(id)].relevance += idf * tf;
					}

				}
			}
		}

		for (const string& word : query_words.min_words) {

			if (docs_content_.count(word)) {

				for (const auto& [id_doc, tf] : docs_content_.at(word)) {

					if (match_ids.count(id_doc) && match_ids.at(id_doc) != -1) {
						int vec_id = match_ids.at(id_doc);

						match_docs[vec_id] = match_docs[match_docs.size() - 1];
						match_ids[id_doc] = -1;
						match_ids[match_docs[vec_id].id] = vec_id;
						match_docs.resize(match_docs.size() - 1);

					}
				}
			}
		}

		return match_docs;
	}

	vector<string> SplitIntoWords(const string& text, bool ignore_stop = false) const {

		vector<string> words;
		string word;

		for (const char c : text) {

			if (c == ' ') {
				if (!word.empty()) {
					if (!ignore_stop || (ignore_stop && !stop_words.count(word))) {
						words.push_back(word);
					}
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

	// fields
	set<string> stop_words;
	vector<Document> relevance;
	map<string, map<int, double>> docs_content_;
	int docs_count_ = 0;
	vector<int> docs_size_;
};

const SearchServer CreateSearchServer()
{
	const string stop_words_joined = ReadLine();
	const int document_count = ReadLineWithNumber();

	SearchServer searcher;
	searcher.SetStopWords(stop_words_joined);

	for (int document_id = 0; document_id < document_count; ++document_id) {
		searcher.AddDocument(document_id, ReadLine());
	}

	return searcher;
}