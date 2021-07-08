#include "test_example_functions.h"

#include <iostream>
#include <execution>

using namespace std;

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

template <typename T>
void RunTestImpl(const T& t, const string& t_str) {
    t();
    cerr << t_str << " OK" << endl;
}

bool InTheVicinity(const double d1, const double d2, const double delta) {
    return abs(d1 - d2) < delta;
}

void TestAddDocument() {
	const int doc_id1 = 10;
	const int doc_id2 = 20;
	string_view content = "cat in the city";
	const vector<int> ratings = {1, 2, 5, 4};

	{
		SearchServer server("in the"sv);
		server.AddDocument(doc_id1, content, DocumentStatus::ACTUAL, ratings);
		server.AddDocument(doc_id2, content, DocumentStatus::BANNED, ratings);
		ASSERT_EQUAL(server.GetDocumentCount(), 2);

		const auto found_docs = server.FindTopDocuments("cat"s);
		ASSERT_EQUAL(found_docs.size(), 1u);
		ASSERT_EQUAL(found_docs.at(0).id, doc_id1);
		ASSERT_EQUAL(found_docs.at(0).rating, 3);

		for (auto doc : server.FindTopDocuments("fun"s))
			cout << doc << endl;
		ASSERT_HINT(server.FindTopDocuments("fun"s).empty(), "Must be empty if added docs have no requested words"s);
	}
}


// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
    	SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

void TestMinusWords() {
		const int doc_id1 = 10;
		const string content1 = "brown cat in the city"s;
		const vector<int> ratings1 = {1, 2, 5, 4};

		const int doc_id2 = 20;
		const string content2 = "bear with brown fur in the city"s;
		const vector<int> ratings2 = {1, 2, 3};

		{
			SearchServer server(""s);
			server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
			server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
			//Исключим первый документ
			const auto found_docs_1 = server.FindTopDocuments("city -cat"s);
			ASSERT_EQUAL(found_docs_1.size(), 1);
			ASSERT_EQUAL(found_docs_1.at(0).id, doc_id2);
			ASSERT_EQUAL(found_docs_1.at(0).rating, 2);

			//Исключим второй документ
			const auto found_docs_2 = server.FindTopDocuments("city -bear"s);
			ASSERT_EQUAL(found_docs_2.size(), 1);
			ASSERT_EQUAL(found_docs_2.at(0).id, doc_id1);
			ASSERT_EQUAL(found_docs_2.at(0).rating, 3);

			//Исключим оба документа
			const auto found_docs_3 = server.FindTopDocuments("brown -city"s);
			ASSERT_HINT(found_docs_3.empty(), "found docs must be empty if minus words are in there"s);

			//Проверка на отсутствие влияния минус-слова, к-ого нет в док-ах
			const auto found_docs_4 = server.FindTopDocuments("brown -dog"s);
			ASSERT_EQUAL(found_docs_4.size(), 2);
			ASSERT_EQUAL(found_docs_4.at(0).id, doc_id1);
			ASSERT_EQUAL(found_docs_4.at(0).rating, 3);
			ASSERT_EQUAL(found_docs_4.at(1).id, doc_id2);
			ASSERT_EQUAL(found_docs_4.at(1).rating, 2);

		}
}

void TestMatchDocumentStatus(const SearchServer& server, const int id, const DocumentStatus status) {

	{
		vector <string_view> words_1;
		DocumentStatus status_1;
		tie(words_1, status_1) = server.MatchDocument("cat -city"sv, id);
		ASSERT_HINT(words_1.empty(), "found docs must be empty if minus words are in there"s);
		ASSERT_EQUAL_HINT(static_cast<int>(status_1), static_cast<int>(status), "status of found doc must be the same"s);
	}
	{

		vector <string_view> words_1;
		DocumentStatus status_1;
		tie(words_1, status_1) = server.MatchDocument("cat city -fake"sv, id);

		ASSERT_EQUAL(words_1.size(), 2);
		ASSERT_EQUAL(words_1[0], "cat"sv);
		ASSERT_EQUAL(words_1[1], "city"sv);
		ASSERT_EQUAL_HINT(static_cast<int>(status_1), static_cast<int>(status), "status of found doc must be the same"s);
	}
}

// Матчинг документов
void TestMatchDocument() {
	SearchServer server("in"sv);
    const int id_1 = 1;
    const int id_2 = 2;
    const int id_3 = 3;
    const int id_4 = 4;
    const vector<int> ratings = {1, 2, 3, 5, 4};
    const string_view content = "cat in the city"sv;

    server.AddDocument(id_1, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(id_2, content, DocumentStatus::BANNED, ratings);
    server.AddDocument(id_3, content, DocumentStatus::IRRELEVANT, ratings);
    server.AddDocument(id_4, content, DocumentStatus::REMOVED, ratings);

    TestMatchDocumentStatus(server, id_1, DocumentStatus::ACTUAL);
    TestMatchDocumentStatus(server, id_2, DocumentStatus::BANNED);
    TestMatchDocumentStatus(server, id_3, DocumentStatus::IRRELEVANT);
    TestMatchDocumentStatus(server, id_4, DocumentStatus::REMOVED);
}

void TestSortByRelevance() {

	SearchServer server(""s);
	const double EPSILON = 1e-6;
	server.AddDocument(0, "cat in the city"sv,        DocumentStatus::ACTUAL, {8, -3});
	server.AddDocument(1, "dog on the floor"sv,       DocumentStatus::ACTUAL, {7, 2, 7});
	server.AddDocument(2, "big elefant with a cat"sv, DocumentStatus::ACTUAL, {5, -12, 2, 1});
	server.AddDocument(3, "cat with blue eyes"sv,        DocumentStatus::ACTUAL, {8, -3});
	server.AddDocument(4, "pink parrot"sv,       DocumentStatus::ACTUAL, {7, 2, 7});
	server.AddDocument(5, "fat rat"sv, DocumentStatus::ACTUAL, {5, -12, 2, 1});

	const auto found_docs = server.FindTopDocuments("cat"s);

	for(size_t i = 0; i + 1 < found_docs.size(); ++i)
		ASSERT_HINT(found_docs.at(i).relevance > found_docs.at(i+1).relevance ||
				abs(found_docs.at(i).relevance - found_docs.at(i+1).relevance) <= EPSILON,
				"Docs must be sorted by relevance or by rating if relevances are equal"s);
}

void TestComputeDocumentRating() {

	const string content = "brown cat in the city"s;

	{
		SearchServer server(""s);
		server.AddDocument(1, content, DocumentStatus::ACTUAL, {0});
		server.AddDocument(2, content, DocumentStatus::ACTUAL, {-1,-9,1});
		server.AddDocument(3, content, DocumentStatus::ACTUAL, {1,2,6});
		server.AddDocument(4, content, DocumentStatus::ACTUAL, {-2,-1,0});
		server.AddDocument(6, content, DocumentStatus::ACTUAL, {1,3});

		const auto found_docs = server.FindTopDocuments(content);
		ASSERT_EQUAL(found_docs.at(0).rating, 3);
		ASSERT_EQUAL(found_docs.at(1).rating, 2);
		ASSERT_EQUAL(found_docs.at(2).rating, 0);
		ASSERT_EQUAL(found_docs.at(3).rating, -1);
		ASSERT_EQUAL(found_docs.at(4).rating, -3);

        for(size_t i = 0; i + 1 < found_docs.size(); ++i)
            ASSERT_HINT(found_docs.at(i).rating > found_docs.at(i+1).rating, "Ratings must be sorted ascending"s);
	}
}

void TestPredicateFilter() {
	const string content = "cat in the city"s;

	SearchServer server(""s);
	server.AddDocument(1, content,        DocumentStatus::ACTUAL, {8, -3}); //2
	server.AddDocument(2, content,       DocumentStatus::ACTUAL, {7, 2, 7});//5
	server.AddDocument(3, content,        DocumentStatus::BANNED, {8, -3, 1, 5});//2
	server.AddDocument(4, content,       DocumentStatus::BANNED, {7, 1});//4
	server.AddDocument(5, content, DocumentStatus::BANNED, {5, -12, 2, 1});//-4

	{
	const auto found_docs = server.FindTopDocuments(content,
			[](int document_id, DocumentStatus status, int rating) {
				(void)rating; (void)status; return document_id % 2 == 0; });
	ASSERT_EQUAL(found_docs.size(), 2);
	ASSERT_EQUAL(found_docs.at(0).id, 2);
	ASSERT_EQUAL(found_docs.at(1).id, 4);
	}

	{
	const auto found_docs = server.FindTopDocuments(content,
			[](int document_id, DocumentStatus status, int rating) {
				(void)document_id; (void)status; return rating >= 2; });
	ASSERT_EQUAL(found_docs.size(), 4);
	ASSERT_EQUAL(found_docs.at(0).id, 2);
	ASSERT_EQUAL(found_docs.at(1).id, 4);
	}
}

void TestFindDocumentByStatus() {
	const string content = "cat in the city"s;

	SearchServer server(""s);
	server.AddDocument(0, content,        DocumentStatus::ACTUAL, {8, -3});
	server.AddDocument(1, content,       DocumentStatus::ACTUAL, {7, 2, 7});
	server.AddDocument(2, content, DocumentStatus::IRRELEVANT, {5, -12, 2, 1});
	server.AddDocument(3, content,        DocumentStatus::BANNED, {8, -3});
	server.AddDocument(4, content,       DocumentStatus::BANNED, {7, 2, 7});
	server.AddDocument(5, content, DocumentStatus::BANNED, {5, -12, 2, 1});

	const auto found_actual_docs = server.FindTopDocuments(content);
	ASSERT_EQUAL(found_actual_docs.size(), 2);

	const auto found_irrelevant_docs = server.FindTopDocuments(content, DocumentStatus::IRRELEVANT);
	ASSERT_EQUAL(found_irrelevant_docs.size(), 1);

	const auto found_banned_docs = server.FindTopDocuments(content, DocumentStatus::BANNED);
	ASSERT_EQUAL(found_banned_docs.size(), 3);

	const auto found_removed_docs = server.FindTopDocuments(content, DocumentStatus::REMOVED);
	ASSERT_HINT(found_removed_docs.empty(), "Found docs must be empty if theres no this status"s);
}

void TestComputeDocumentRelevance() {
	double EPSILON = 1e-6;
	vector<double> relevances = {0.650672, 0.274653, 0.101366};

	SearchServer server("и в на"s);
	server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
	server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});

	const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s);
	for (size_t i = 0; i < found_docs.size(); ++i)
		ASSERT_HINT(abs(relevances[i] - found_docs.at(i).relevance) <= EPSILON, "Test relevances must be equal with computed relevances"s);
}

void TestRemoveDocument() {
    const double delta = 1e-6;
    SearchServer server("and with as"sv);

	server.AddDocument(2, "funny pet with curly hair"sv, DocumentStatus::ACTUAL, { 1, 2 });
	server.AddDocument(4, "kind dog bite fat rat"sv, DocumentStatus::ACTUAL, { 1, 2 });
	server.AddDocument(6, "fluffy snake or cat"sv, DocumentStatus::ACTUAL, { 1, 2 });

	server.AddDocument(1, "funny pet and nasty rat"sv, DocumentStatus::ACTUAL, { 7, 2, 7 });
    // nasty tf = 1/4
	server.AddDocument(3, "angry rat with black hat"sv, DocumentStatus::ACTUAL, { 1, 2 });
    // black tf = 1/4
	server.AddDocument(5, "fat fat cat"sv, DocumentStatus::ACTUAL, { 1, 2 });
    // cat tf = 1/3
	server.AddDocument(7, "sharp as hedgehog"sv, DocumentStatus::ACTUAL, { 1, 2 });
    // sharp tf = 1/2
    // kind - doesn't occur
    // nasty - log(4)
    // black - log(4)
    // cat - log(4)
    // sharp - log(4)

    // 7 - 1/2 * log(4) = 0.6931471805599453
    // 5 - 1/3 * log(4) = 0.46209812037329684
    // 1 - 1/4 * log(4) = 0.34657359027997264
    // 3 - 1/4 * log(4) = 0.34657359027997264

    server.RemoveDocument(0);
    server.RemoveDocument(8);

    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 7, "Nothing has been removed, yet!"s);

    server.RemoveDocument(2);
    server.RemoveDocument(4);
    server.RemoveDocument(6);

    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 4, "3 documents have been removed"s);

    // Check document_data_
    ASSERT_HINT(server.GetWordFrequencies(2).empty(), "Server doesn't has id = 2, result must be empty"s);
    ASSERT_HINT(server.GetWordFrequencies(4).empty(), "Server doesn't has id = 4, result must be empty"s);
    ASSERT_HINT(server.GetWordFrequencies(6).empty(), "Server doesn't has id = 6, result must be empty"s);

    // Check document_ids_
    for (int id : server) {
        ASSERT_HINT(id % 2 == 1, "Only odd ids has been left"s);
    }

    // Check word_to_document_freqs_
    const auto docs = server.FindTopDocuments("kind nasty black sharp cat"s);
    ASSERT_HINT(docs.size() == 4, "All documents must be found"s);

    ASSERT_EQUAL_HINT(docs.at(0).id, 7, "Max relevance has doc with id 7"s);
    ASSERT_HINT(InTheVicinity(docs.at(0).relevance, 0.6931471805599453, delta), "Wrong relevance"s);

    ASSERT_EQUAL_HINT(docs.at(1).id, 5, "Second relevance has doc with id 5"s);
    ASSERT_HINT(InTheVicinity(docs.at(1).relevance, 0.46209812037329684, delta), "Wrong relevance"s);

    ASSERT_EQUAL_HINT(docs.at(2).id, 1, "Third relevance has doc with id 1"s);
    ASSERT_HINT(InTheVicinity(docs.at(2).relevance, 0.34657359027997264, delta), "Wrong relevance"s);

    ASSERT_EQUAL_HINT(docs.at(3).id, 3, "Forth relevance has doc with id 3"s);
    ASSERT_HINT(InTheVicinity(docs.at(3).relevance, 0.34657359027997264, delta), "Wrong relevance"s);
}

void TestRelevance() {
	//Напомню сам себе -  tf слова оценивает, сколько раз оно
	//встречается в данном документе, т.е. если слово заяц
	// встречается в документе из 4 слов два раза, то
	// tf = 2/4=0.5
	// idf слова вычиляют так:
	// Количество всех документов делят на количество тех, где встречается слово.
	// Потом берут логарифм от результата
	// Для каждого слова умножаем tf на idf и потом просуммируем все результаты
	const double EPSILON = 1e-6;
	//вначале у нас один документ, idf = log(1/1)=0
	{
		SearchServer server(""s);
		server.AddDocument(1, "aaa bbb", DocumentStatus::ACTUAL, { 1 });
		const auto found_docs = server.FindTopDocuments("aaa");
		ASSERT(abs(found_docs[0].relevance) < EPSILON);
	}
	//Проверим на "классическом примере"
	{
		SearchServer server("и в на"s);

		vector<int> ratings = { 1, 2, 3 };

		server.AddDocument(0, "белый кот и модный ошейник"sv,
				DocumentStatus::ACTUAL, ratings);
		server.AddDocument(1, "пушистый кот пушистый хвост"sv,
				DocumentStatus::ACTUAL, ratings);
		server.AddDocument(2, "ухоженный пёс выразительные глаза"sv,
				DocumentStatus::ACTUAL, ratings);
		server.AddDocument(3, "ухоженный скворец евгений"sv,
				DocumentStatus::BANNED, ratings);

		//ACTUAL:
		// { document_id = 1, relevance = 0.866434, rating = 5 }
		//{ document_id = 0, relevance = 0.173287, rating = 2 }
		// { document_id = 2, relevance = 0.173287, rating = -1 }
		// BANNED:
		// { document_id = 3, relevance = 0.231049, rating = 9 }

		const auto found_docs = server.FindTopDocuments(
				"пушистый ухоженный кот"sv);

		const Document &doc0 = found_docs[0];
		const Document &doc1 = found_docs[1];
		const Document &doc2 = found_docs[2];
		const double error1 = abs(doc0.relevance - 0.866434);
		const double error2 = abs(doc1.relevance - 0.173287);
		const double error3 = abs(doc2.relevance - 0.173287);

		ASSERT(error1 < EPSILON);
		ASSERT(error2 < EPSILON);
		ASSERT(error3 < EPSILON);
	}
}

void TestSearchServer() {
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortByRelevance);
    RUN_TEST(TestFindDocumentByStatus);
    RUN_TEST(TestComputeDocumentRelevance);
   // RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestPredicateFilter);
    RUN_TEST(TestComputeDocumentRating);
    RUN_TEST(TestRelevance);

}



