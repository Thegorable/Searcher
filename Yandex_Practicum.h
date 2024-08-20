#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
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
private:
	struct query { set<string> plus_words; set<string> min_words; };

public:

	void AddDocument(int doc_id, const string& text)
	{
		const vector<string> words = SplitIntoWordsNoStop(text);
		double doc_size = DOUBLE(words.size());

		for (const string& word : words)
		{
			if (docs_content.count(word))
			{
				docs_content[word][doc_id] += 1.0 / doc_size;
			}
			else
			{
				docs_content[word] = map<int, double>{ {doc_id, 1.0 / doc_size} };
			}
		}

		docs_size_.push_back(words.size());
		docs_count_++;
	}

	query ParseQuery(const string& text) const
	{
		query query_words;
		
		for (const string& word : SplitIntoWordsNoStop(text)) {
			if (word.at(0) == '-')
			{
				query_words.min_words.insert(word.substr(1));
			}
			else
			{
				query_words.plus_words.insert(word);
			}
		}

		return query_words;
	}

	void SetStopWords(const string& text) {
		if (text.size())
		{
			for (const string& word : SplitIntoWords(text)) {
				stop_words.insert(word);
			}
		}
	}
	
	vector<Document> FindTopDocuments(const string& raw_query) const 
	{
		const query query_words = ParseQuery(raw_query);
		vector<Document> matched_documents = FindAllDocuments(query_words);

		sort(matched_documents.begin(), matched_documents.end(), [](const Document& doc1, const Document& doc2) -> bool {
				return doc1.relevance > doc2.relevance;
			});

		if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
			matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
		}

		return matched_documents;
	}

private: // methods
	
	double calculate_IDF(const string& word) const
	{
		if (docs_content.count(word))
		{
			double count = DOUBLE (docs_count_) / DOUBLE(docs_content.at(word).size());
			return log(count);
		}
		else
		{
			return 0.0;
		}
	}
	
	vector<Document> FindAllDocuments(const query& query_words) const
	{
		map<int, double> document_to_relevance;
		for (const string& word : query_words.plus_words)
		{
			double idf = calculate_IDF(word);
			
			if (docs_content.count(word) && idf)
			{
				for (const auto& [id, count] : docs_content.at(word))
				{
					if (document_to_relevance.count(id))
					{
						document_to_relevance[id] += idf * docs_content.at(word).at(id);
					}
					else
					{
						document_to_relevance[id] = idf * docs_content.at(word).at(id);
					}
				}
			}
		}

		for (const string& word : query_words.min_words)
		{
			if (docs_content.count(word))
			{
				for (const auto& [id, count] : docs_content.at(word))
				{
					if (document_to_relevance.count(id))
					{
						document_to_relevance[id] = 0.0;
					}
				}
			}
		}

		vector<Document> matched_documents;
		for (const auto& [id, relevance] : document_to_relevance)
		{
			if (relevance)
			{
				matched_documents.push_back({ id, relevance });
			}
		}

		return matched_documents;
	}
	

private: // fields
	set<string> stop_words;
	vector<Document> relevance;
	map<string, map<int, double>> docs_content;
	int docs_count_ = 0;
	vector<int> docs_size_;

private: // methods

	vector<string> SplitIntoWords(const string& text) const
	{
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

	vector<string> SplitIntoWordsNoStop(const string& text) const
	{
		vector<string> words;
		for (const string& word : SplitIntoWords(text)) {
			if (stop_words.count(word) == 0) {
				words.push_back(word);
			}
		}
		return words;
	}

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