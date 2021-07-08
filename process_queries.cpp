#include "process_queries.h"
#include <execution>
#include <iostream>
using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
	vector<vector<Document>> process_queries(queries.size());
	transform(execution::par, queries.begin(), queries.end(), process_queries.begin(), [&search_server](string query) {
		return search_server.FindTopDocuments(query);
		});

	return process_queries;
}

vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries) {
	vector<Document> documents;
	for (const auto& local_documents : ProcessQueries(search_server, queries)) {
		documents.insert(documents.end(), local_documents.begin(), local_documents.end());
	}
	return documents;
}