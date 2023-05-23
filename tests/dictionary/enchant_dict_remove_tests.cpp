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

#include <UnitTest++/UnitTest++.h>
#include <enchant.h>
#include "EnchantDictionaryTestFixture.h"

static bool dictCheckCalled;

static int
MockDictionaryCheck (EnchantDict * dict, const char *const word, size_t len)
{
    dict;
    dictCheckCalled = true;
    if(strncmp("hello", word, len)==0)
    {
        return 0; //good word
    }
    return 1; // bad word
}


static EnchantDict* MockProviderRequestAddToExcludeMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestBasicMockDictionary(me, tag);
    dict->check = MockDictionaryCheck;
    return dict;
}

static void DictionaryAddToExclude_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestAddToExcludeMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryRemove_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryRemove_TestFixture():
            EnchantDictionaryTestFixture(DictionaryAddToExclude_ProviderConfiguration)
    { 
        dictCheckCalled = false;
    }
};

static EnchantDict* MockProviderRequestCheckMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->check = MockDictionaryCheck;
    return dict;
}

static void DictionaryCheck_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestCheckMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryRemoveNotImplemented_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryRemoveNotImplemented_TestFixture():
            EnchantDictionaryTestFixture(DictionaryCheck_ProviderConfiguration)
    { 
        dictCheckCalled = false;
    }
};
/**
 * enchant_dict_remove
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your exclude dictionary and 
 *        remove from the personal dictionary, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 */


/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_WordNoLongerExistsInDictionary)
{
    CHECK(IsWordInDictionary("hello"));
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_WordNoLongerExistsInSession)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_WordAddedToEnchantExcludeFile)
{
    CHECK(!ExcludeFileHasContents());
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(ExcludeFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_WordRemovedFromEnchantPwlFile)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(PersonalWordListFileHasContents());
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_ProviderNotAskedIfWordExistsInDictionary)
{
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_NotGivenAsSuggestion)
{
    enchant_dict_remove(_dict, "aelo", -1);

    std::vector<std::string> suggestions = GetSuggestions("helo");
    CHECK_EQUAL(3, suggestions.size());
    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo",1), suggestions,
                      std::min(static_cast<std::vector<std::string>::size_type>(3),
                               suggestions.size()));
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_ThenAddedToSession_GivenAsSuggestion)
{
    enchant_dict_remove(_dict, "aelo", -1);
    enchant_dict_add_to_session(_dict, "aelo", -1);

    std::vector<std::string> suggestions = GetSuggestions("helo");
    CHECK_EQUAL(4, suggestions.size());
    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo"), suggestions, suggestions.size());
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_ThenAddedToSession_WhenSessionOverStillExcluded)
{
    enchant_dict_remove(_dict, "aelo", -1);
    enchant_dict_add_to_session(_dict, "aelo", -1);

    ReloadTestDictionary(); // to see what actually persisted

    std::vector<std::string> suggestions = GetSuggestions("helo");
    CHECK_EQUAL(3, suggestions.size());
    CHECK_ARRAY_EQUAL(GetExpectedSuggestions("helo",1), suggestions,
                      std::min(static_cast<std::vector<std::string>::size_type>(3),
                               suggestions.size()));
}

/* since the broker pwl file is a read/write file (there is no readonly dictionary in addition)
 * there is no need for complementary exclude file to add a word to. The word just needs to be
 * removed from the broker pwl file
 */
TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_OnBrokerPwl_RemovedFromBrokerPwlFile)
{
    CHECK(!BrokerPWLFileHasContents());
    enchant_dict_add(_pwl, "personal", -1);
    CHECK(BrokerPWLFileHasContents());

    enchant_dict_remove(_pwl, "personal", -1);
    CHECK(!BrokerPWLFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_OnBrokerPwl_NotAddedToOtherPwlFile)
{
    CHECK(!BrokerPWLFileHasContents());

    enchant_dict_remove(_pwl, "personal", -1);
    CHECK(!ExcludeFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemove_TestFixture,
             EnchantDictionaryRemove_IsPermanent)
{
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));

    ReloadTestDictionary();

    CHECK(!IsWordInDictionary("hello"));
}


TEST_FIXTURE(EnchantDictionaryRemove_TestFixture, 
             EnchantDictionaryRemove_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_remove(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryRemoveNotImplemented_TestFixture,
             EnchantDictionaryRemoveNotImplemented_WordAddedToExcludeFile)
{
    CHECK(!ExcludeFileHasContents());
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(ExcludeFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryRemoveNotImplemented_TestFixture,
             EnchantDictionaryRemoveNotImplemented_StillExcluded)
{
    CHECK(IsWordInDictionary("hello"));
    enchant_dict_remove(_dict, "hello", -1);
    CHECK(!IsWordInDictionary("hello"));
}
