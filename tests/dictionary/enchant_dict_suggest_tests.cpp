/* Copyright (c) 2007 Eric Scott Albright
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define NOMINMAX //don't want windows to collide with std::min
#include <UnitTest++.h>
#include <enchant.h>
#include <vector>
#include <algorithm>

#include "EnchantDictionaryTestFixture.h"

static bool dictSuggestCalled;
static bool providerFreeStringListCalled;
std::string suggestWord;

static enum SuggestBehavior{
    returnNull,
    returnZero,
    returnFour,
    returnFourOneInvalidUtf8,
    returnFianceNfc
} suggestBehavior;

struct EnchantDictionarySuggestTestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionarySuggestTestFixtureBase(ConfigureHook userConfiguration):
            EnchantDictionaryTestFixture(userConfiguration)
    { 
        dictSuggestCalled = false;
        providerFreeStringListCalled = false;
        _suggestions = NULL;
        suggestWord = std::string();
        suggestBehavior = returnFour;
    }
    //Teardown
    ~EnchantDictionarySuggestTestFixtureBase()
    {
        FreeStringList(_suggestions);
    }

    char** _suggestions;
};

static char **
MyMockDictionarySuggest (EnchantDict * dict, const char *const word, size_t len, size_t * out_n_suggs)
{
    dictSuggestCalled = true;
    suggestWord = std::string(word,len);
    *out_n_suggs = 0;
    char **sugg_arr = NULL;

    switch(suggestBehavior)
    {
        case returnNull:
            sugg_arr = NULL;
            break;
        case returnZero:
            sugg_arr = g_new0 (char *, *out_n_suggs + 1);
            break;
        case returnFianceNfc:
            *out_n_suggs = 1;
            sugg_arr = g_new0 (char *, *out_n_suggs + 1);
            sugg_arr[0] = g_strdup ("fianc\xc3\xa9");  // c3 a9 = utf8 for u00e9 = Latin small letter e with acute
            break;
        case returnFour:
            sugg_arr = MockDictionarySuggest(dict, word, len, out_n_suggs);
            break;
        case returnFourOneInvalidUtf8:
            sugg_arr = MockDictionarySuggest(dict, word, len, out_n_suggs);
            g_free(sugg_arr[0]);
            sugg_arr[0] = g_strdup ("\xa5\xf1\x08");
            break;
    }

    return sugg_arr;
}

static void
MyMockProviderFreeStringList (EnchantProvider * provider, char **str_list)
{
    providerFreeStringListCalled = true;
    return MockProviderFreeStringList(provider, str_list);
}

static void
MyMockProviderAlternativeFreeStringList (EnchantProvider * provider, char **str_list)
{
    return MockProviderFreeStringList(provider, str_list);
}

static EnchantDict* MockProviderRequestSuggestMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = MyMockDictionarySuggest;
    return dict;
}

static void DictionarySuggest_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestSuggestMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
     me->free_string_list = MyMockProviderFreeStringList;
}




struct EnchantDictionarySuggest_TestFixture : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggest_TestFixture():
            EnchantDictionarySuggestTestFixtureBase(DictionarySuggest_ProviderConfiguration)
    { }
};




static EnchantDict* MockProviderRequestNoSuggestMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = NULL;
    return dict;
}

static void DictionaryNoSuggest_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestNoSuggestMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
     me->free_string_list = MyMockProviderFreeStringList;
}

struct EnchantDictionarySuggestNotImplemented_TestFixture : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggestNotImplemented_TestFixture():
            EnchantDictionarySuggestTestFixtureBase(DictionaryNoSuggest_ProviderConfiguration)
    { }
};


static EnchantDict* MockProviderRequestAlternativeFreeMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = MyMockDictionarySuggest;
    return dict;
}

static void DictionaryAlternativeFree_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestAlternativeFreeMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
     me->free_string_list = MyMockProviderAlternativeFreeStringList;
}

struct EnchantDictionaryFreeNotImplemented_TestFixture : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionaryFreeNotImplemented_TestFixture():
            EnchantDictionarySuggestTestFixtureBase(DictionaryAlternativeFree_ProviderConfiguration)
    { }
};




/**
 * enchant_dict_suggest
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to find suggestions for, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 * @out_n_suggs: The location to store the # of suggestions returned, or %null
 *
 * Will return an %null value if any of those pre-conditions
 * are not met.
 *
 * Returns: A %null terminated list of UTF-8 encoded suggestions, or %null
 */
/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_LenComputed)
{
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(std::string("helo"), suggestWord);
    CHECK_EQUAL(4, cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, std::min((size_t)4,cSuggestions));
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_LenSpecified)
{
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helodisregard me", 4, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(std::string("helo"), suggestWord);
    CHECK_EQUAL(4, cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, std::min((size_t)4,cSuggestions));
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_StringListFreed)
{
    size_t cSuggs;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggs);
    CHECK(dictSuggestCalled);
    CHECK(providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullOutputSuggestionCount)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, NULL);
    CHECK(_suggestions);
    CHECK(dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_InBrokerPwlSession)
{
    enchant_dict_add(_pwl, "hello", -1);
    _suggestions = enchant_dict_suggest(_pwl, "helo", -1, NULL);
    CHECK(_suggestions);
    CHECK(!dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_SuggestionsFromPersonal_addedToEnd)
{
    size_t cSuggestions;
    enchant_dict_add(_dict, "hello", -1);
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(5, cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    std::vector<std::string> expected = GetExpectedSuggestions("helo");
    expected.push_back("hello");

    CHECK_ARRAY_EQUAL(expected, suggestions, std::min((size_t)5,cSuggestions));
}


TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_DuplicateSuggestionsFromPersonal_notIncluded)
{
    size_t cSuggestions;

    enchant_dict_add(_dict, "aelo", -1);
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(4, cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, std::min((size_t)4,cSuggestions));
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_SuggestionExcluded_Null)
{
    suggestBehavior = returnFianceNfc;
    RemoveWordFromDictionary(Convert(L"fianc\xe9"));  // u00e9 = Latin small letter e with acute

    _suggestions = enchant_dict_suggest(_dict, "fiance", -1, NULL);
    CHECK(!_suggestions);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture, 
             EnchantDictionarySuggest_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    _suggestions = enchant_dict_suggest(_dict, "helo", -1, NULL);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullDictionary_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(NULL, "helo", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullWord_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, NULL, -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_EmptyWord_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, "", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_WordSize0_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", 0, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_InvalidUtf8Correction_DoNothing)
{
    _suggestions = enchant_dict_suggest(_dict, "\xa5\xf1\x08", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}


TEST_FIXTURE(EnchantDictionarySuggestNotImplemented_TestFixture,
             EnchantDictionarySuggestNotImplemented_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionaryFreeNotImplemented_TestFixture,
             EnchantDictionaryFreeNotImplemented_Suggestions_FreeNotCalled)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, NULL);

    CHECK(_suggestions);
    CHECK(dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_EmptySuggestionList_FreeCalled)
{
    suggestBehavior = returnZero;
    size_t cSuggs;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggs);
    CHECK(!_suggestions);
    CHECK_EQUAL(0, cSuggs);
    CHECK(dictSuggestCalled);
    CHECK(providerFreeStringListCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullSuggestionList_FreeNotCalled)
{
    suggestBehavior = returnNull;
    size_t cSuggs;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggs);
    CHECK(!_suggestions);
    CHECK_EQUAL(0, cSuggs);
    CHECK(dictSuggestCalled);
    CHECK(!providerFreeStringListCalled);
}


TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_SuggestionListWithInvalidUtf8_InvalidSuggestionIgnored_FreeCalled)
{
    suggestBehavior = returnFourOneInvalidUtf8;
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK(dictSuggestCalled);
    CHECK(providerFreeStringListCalled);

    CHECK_EQUAL(3, cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo",1), suggestions, std::min((size_t)3,cSuggestions));
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_WordNfcInDictionaryNfdInPwl_ReturnsFromDict)
{
    suggestBehavior = returnFianceNfc;

    ExternalAddWordToDictionary(Convert(L"fiance\x301")); // NFD u0301 = Combining acute accent

    ReloadTestDictionary();

    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "fiance", -1, &cSuggestions);
    CHECK(_suggestions);

    CHECK_EQUAL(1, cSuggestions);
    CHECK_EQUAL(Convert(L"fianc\xe9"), _suggestions[0]);
}

TEST_FIXTURE(EnchantDictionarySuggestNotImplemented_TestFixture,
             EnchantDictionarySuggest_WordInDictionaryAndExclude_NotInSuggestions)
{
    ExternalAddWordToExclude("hello");
    ExternalAddWordToDictionary("hello");

    ReloadTestDictionary();

    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(!_suggestions);

    CHECK_EQUAL(0, cSuggestions);
}

