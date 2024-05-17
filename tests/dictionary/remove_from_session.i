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

#ifndef EnchantDictionaryRemoveFromSession_TestFixture
#error EnchantDictionaryRemoveFromSession_TestFixture must be defined as the testfixture class to run these tests against
#endif

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_WordNotAddedToEnchantExcludeFile)
{
    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK(!ExcludeFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_WordRemovedFromSession)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK(!IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_CalledTwice)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    enchant_dict_remove_from_session(_dict, "hello", -1);
    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK(!IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_WordExcludedFromDictionary)
{
    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_InBrokerSession_WordExcludedFromBrokerSession)
{
    enchant_dict_add_to_session(_pwl, "hello", -1);
    CHECK(enchant_dict_is_added(_pwl, "hello", -1));
    enchant_dict_remove_from_session(_pwl, "hello", -1);
    CHECK(!enchant_dict_is_added(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_IsNotPermanent)
{
    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));

    ReloadTestDictionary();

    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_NotGivenAsSuggestion)
{
    enchant_dict_remove_from_session(_dict, "aelo", -1);

    std::vector<std::string> suggestions = GetSuggestions("helo");
    CHECK_EQUAL(3 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), suggestions.size());
    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo",1), suggestions, 3);
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_ThenAddedToSession_GivenAsSuggestion)
{
    enchant_dict_remove_from_session(_dict, "aelo", -1);
    enchant_dict_add_to_session(_dict, "aelo", -1);

    std::vector<std::string> suggestions = GetSuggestions("helo");
    CHECK_EQUAL(4 * (std::count(languageTag.begin(), languageTag.end(), ',') + 1), suggestions.size());
    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, 4);
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture, 
             EnchantDictionaryRemoveFromSession_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_remove_from_session(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_NullDictionary_NotRemoved)
{
    enchant_dict_remove_from_session(NULL, "hello", -1);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_NullWord_NotRemoved)
{
    enchant_dict_remove_from_session(_dict, NULL, -1);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_EmptyWord_NotRemoved)
{
    enchant_dict_remove_from_session(_dict, "", -1);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_WordSize0_NotRemoved)
{
    enchant_dict_remove_from_session(_dict, "hello", 0);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemoveFromSession_TestFixture,
             EnchantDictionaryRemoveFromSession_InvalidUtf8Word_NotRemoved)
{
    enchant_dict_remove_from_session(_dict, "\xa5\xf1\x08", -1);
    CHECK(IsWordInDictionary("hello"));
}
