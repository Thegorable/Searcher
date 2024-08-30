#include "Yandex_Practicum.h"

using namespace std;

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating
        << " }"s << endl;
}

int main() {

    cout.precision(20);
    cout << EPSILON << '\n';
    SearchServer search_server;
    search_server.SetStopWords("and in on"s);

    search_server.AddDocument(0, "white cat and cool collar"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "fluffy cat fluffy tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "groomed dog beautiful eyes"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });

    for (const Document& document : search_server.FindTopDocuments("groomed cat"s)) {
        PrintDocument(document);
    }
}