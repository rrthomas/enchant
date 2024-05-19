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

#ifndef EnchantDictionarySuggest_TestFixture
#error EnchantDictionarySuggest_TestFixture must be defined as the testfixture class to run these tests against
#endif

#ifndef EnchantDictionarySuggestNotImplemented_TestFixture
#error EnchantDictionarySuggestNotImplemented_TestFixture must be defined as the testfixture class to run these tests against
#endif

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_LenComputed)
{
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(std::string("helo"), suggestWord);
    CHECK_EQUAL(4 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), cSuggestions);

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
    CHECK_EQUAL(4 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), cSuggestions);

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
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullOutputSuggestionCount)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, NULL);
    CHECK(_suggestions);
    CHECK(dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_DuplicateSuggestionsFromPersonal_notIncluded)
{
    size_t cSuggestions;

    enchant_dict_add(_dict, "aelo", -1);
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK_EQUAL(4 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), cSuggestions);

    std::vector<std::string> suggestions;
    if(_suggestions != NULL){
        suggestions.insert(suggestions.begin(), _suggestions, _suggestions+cSuggestions);
    }

    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, std::min((size_t)4,cSuggestions));
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_SuggestionExcluded_Empty)
{
    suggestBehavior = returnFianceNfc;
    RemoveWordFromDictionary(Convert(L"fianc\xe9"));  // u00e9 = Latin small letter e with acute

    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "fiance", -1, &cSuggestions);
    CHECK_EQUAL(cSuggestions, 0);
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
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_NullWord_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, NULL, -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_EmptyWord_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, "", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_WordSize0_NullSuggestions)
{
    _suggestions = enchant_dict_suggest(_dict, "helo", 0, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_InvalidUtf8Correction_DoNothing)
{
    _suggestions = enchant_dict_suggest(_dict, "\xa5\xf1\x08", -1, NULL);

    CHECK(!_suggestions);
    CHECK(!dictSuggestCalled);
}


TEST_FIXTURE(EnchantDictionarySuggestNotImplemented_TestFixture,
             EnchantDictionarySuggestNotImplemented_NullSuggestions)
{
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);

    CHECK_EQUAL(cSuggestions, 0);
    CHECK(!dictSuggestCalled);
}

TEST_FIXTURE(EnchantDictionarySuggest_TestFixture,
             EnchantDictionarySuggest_SuggestionListWithInvalidUtf8_InvalidSuggestionIgnored_FreeCalled)
{
    suggestBehavior = returnFourOneInvalidUtf8;
    size_t cSuggestions;
    _suggestions = enchant_dict_suggest(_dict, "helo", -1, &cSuggestions);
    CHECK(_suggestions);
    CHECK(dictSuggestCalled);

    CHECK_EQUAL(3 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), cSuggestions);

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

    CHECK_EQUAL(1 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), cSuggestions);
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
    CHECK_EQUAL(0, cSuggestions);
}

