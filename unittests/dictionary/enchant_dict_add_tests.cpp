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

#include <UnitTest++.h>
#include <enchant.h>
#include "../EnchantDictionaryTestFixture.h"

static bool addToPersonalCalled;
static std::string wordToAdd;

static void
MockDictionaryAddToPersonal (EnchantDict * dict, const char *const word, size_t len)
{
    dict;
    addToPersonalCalled = true;
    wordToAdd = std::string(word, len);
}

static EnchantDict* MockProviderRequestAddToPersonalMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->add_to_personal = MockDictionaryAddToPersonal;
    return dict;
}

static void DictionaryAddToPersonal_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestAddToPersonalMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryAdd_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAdd_TestFixture():
            EnchantDictionaryTestFixture(DictionaryAddToPersonal_ProviderConfiguration)
    { 
        addToPersonalCalled = false;
    }
};

struct EnchantDictionaryAddToPersonalNotImplemented_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToPersonalNotImplemented_TestFixture():
            EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration)
    { 
        addToPersonalCalled = false;
    }
};

/**
 * enchant_dict_add
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your personal dictionary, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 * Remarks: if the word exists in the exclude dictionary, it will be removed from the 
 *          exclude dictionary
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInDictionary)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInSession)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordAddedToEnchantPwlFile)
{
    CHECK(!PersonalWordListFileHasContents());
    enchant_dict_add(_dict, "hello", -1);
    CHECK(PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_PassedOnToProvider_LenComputed)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(addToPersonalCalled);
    CHECK_EQUAL(std::string("hello"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_PassedOnToProvider_LenSpecified)
{
    enchant_dict_add(_dict, "hellodisregard me", 5);
    CHECK(addToPersonalCalled);
    CHECK_EQUAL(std::string("hello"), wordToAdd);
}


TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInSession_StillCallsProvider)
{
    enchant_dict_add_to_session(_dict, "session", -1);
    CHECK(!addToPersonalCalled);

    enchant_dict_add(_dict, "session", -1);
    CHECK(addToPersonalCalled);
    CHECK_EQUAL(std::string("session"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInPersonal_StillCallsProvider)
{
    enchant_dict_add(_dict, "personal", -1);
    addToPersonalCalled=false;
    wordToAdd = std::string();

    enchant_dict_add(_dict, "personal", -1);
    CHECK(addToPersonalCalled);
    CHECK_EQUAL(std::string("personal"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInExclude_RemovedFromExcludeAddedToPersonal)
{
    enchant_dict_remove(_dict, "personal", -1);
    CHECK(ExcludeFileHasContents());
    enchant_dict_add(_dict, "personal", -1);
    CHECK(!ExcludeFileHasContents());
    CHECK(PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_InBrokerPwl)
{
    enchant_dict_add(_pwl, "personal", -1);
    CHECK(!addToPersonalCalled);
    CHECK(!PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_IsPermanent)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInDictionary("hello"));

    ReloadTestDictionary();

    CHECK(IsWordInDictionary("hello"));
}


TEST_FIXTURE(EnchantDictionaryAdd_TestFixture, 
             EnchantDictionaryAdd_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_add(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_NullDictionary_NotAdded)
{
    enchant_dict_add(NULL, "hello", -1);
    CHECK(!addToPersonalCalled);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_NullWord_NotAdded)
{
    enchant_dict_add(_dict, NULL, -1);
    CHECK(!addToPersonalCalled);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_EmptyWord_NotAdded)
{
    enchant_dict_add(_dict, "", -1);
    CHECK(!addToPersonalCalled);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordSize0_NotAdded)
{
    enchant_dict_add(_dict, "hello", 0);
    CHECK(!addToPersonalCalled);
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_InvalidUtf8Word_NotAdded)
{
    enchant_dict_add(_dict, "\xa5\xf1\x08", -1);
    CHECK(!addToPersonalCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToPersonalNotImplemented_TestFixture,
             EnchantDictionaryAddToPersonalNotImplemented_WordAddedToEnchantPwlFile)
{
    CHECK(!PersonalWordListFileHasContents());
    enchant_dict_add(_dict, "hello", -1);
    CHECK(PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAddToPersonalNotImplemented_TestFixture,
             EnchantDictionaryAddToPersonalNotImplemented_NotPassedOnToProvider)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(!addToPersonalCalled);
}
