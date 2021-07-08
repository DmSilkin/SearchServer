#pragma once
#include "search_server.h"
#include <cassert>

using namespace std;

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint);

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& t, const string& t_str);


bool InTheVicinity(const double d1, const double d2, const double delta);

// Проверка метода удаления документа
void TestRemoveDocument();

void TestAddDocument();
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

void TestMinusWords();

void TestMatchDocumentStatus(const SearchServer& server, const int id, const DocumentStatus status);

// Матчинг документов
void TestMatchDocument();

void TestSortByRelevance();

void TestComputeDocumentRating();

void TestPredicateFilter();

void TestFindDocumentByStatus();

void TestComputeDocumentRelevance();

void TestRelevance();

#define RUN_TEST(func) RunTestImpl((func),#func)

void TestSearchServer();
