#include "remove_duplicates.h"
#include <vector>
#include <iostream>
#include <map>
#include <set>
using namespace std;


/*void RemoveDuplicates(SearchServer& search_server) {
	vector <int> duplicate_id;;
	map<set<string>, int> words_to_id;

	for (const int document_id : search_server) {
		set<string> words;
		const auto words_to_freq = search_server.GetWordFrequencies(document_id);
		for (const auto elem : words_to_freq) {
			words.insert(elem.first);
		}
		if(words_to_id.count(words) == 0) {
			words_to_id[words] = document_id;
		}
		else {
			duplicate_id.push_back(document_id);
		}
	}

	for (const int id : duplicate_id) {
		//cout << "Found duplicate document id " << id << endl;
		search_server.RemoveDocument(id);
	}
}
*/