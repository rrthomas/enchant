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

static bool addToSessionCalled;
static std::string wordToAdd;

static void
MockDictionaryAddToSession (EnchantDict * dict, const char *const word, size_t len)
{
    dict;
    addToSessionCalled = true;
    wordToAdd = std::string(word, len);
}

static EnchantDict* MockProviderRequestAddToSessionMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->add_to_session = MockDictionaryAddToSession;
    return dict;
}

static void DictionaryAddToSession_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestAddToSessionMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryAddToSession_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSession_TestFixture():
            EnchantDictionaryTestFixture(DictionaryAddToSession_ProviderConfiguration)
    { 
        addToSessionCalled = false;
    }
};

struct EnchantDictionaryAddToSessionNotImplemented_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSessionNotImplemented_TestFixture():
            EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration)
    { 
        addToSessionCalled = false;
    }
};
/**
 * enchant_dict_add_to_session
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to this spell-checking session, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordNotAddedToEnchantPwlFile)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(!PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_PassedOnToProvider_LenComputed)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(addToSessionCalled);
    CHECK_EQUAL(std::string("hello"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_PassedOnToProvider_LenSpecified)
{
    enchant_dict_add_to_session(_dict, "hellodisregard me", 5);
    CHECK(addToSessionCalled);
    CHECK_EQUAL(std::string("hello"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordExistsInSession_StillCallsProvider)
{
    enchant_dict_add_to_session(_dict, "session", -1);
    addToSessionCalled=false;
    wordToAdd = std::string();

    enchant_dict_add_to_session(_dict, "session", -1);
    CHECK(addToSessionCalled);
    CHECK_EQUAL(std::string("session"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordExistsInPersonal_StillCallsProvider)
{
    enchant_dict_add(_dict, "personal", -1);
    enchant_dict_add_to_session(_dict, "personal", -1);
    CHECK(addToSessionCalled);
    CHECK_EQUAL(std::string("personal"), wordToAdd);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordExistsInExclude_AddedToSessionNotRemovedFromExcludeFile)
{
    enchant_dict_remove(_dict, "personal", -1);

    enchant_dict_add_to_session(_dict, "personal", -1);
    CHECK(IsWordInDictionary("personal"));
    CHECK(ExcludeFileHasContents());
}


TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordAddedToSession)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_InBrokerSession)
{
    enchant_dict_add_to_session(_pwl, "personal", -1);
    CHECK(!addToSessionCalled);
    CHECK(!PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_IsNotPermanent)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));

    ReloadTestDictionary();

    CHECK(!IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture, 
             EnchantDictionaryAddToSession_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_NullDictionary_NotAdded)
{
    enchant_dict_add_to_session(NULL, "hello", -1);
    CHECK(!addToSessionCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_NullWord_NotAdded)
{
    enchant_dict_add_to_session(_dict, NULL, -1);
    CHECK(!addToSessionCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_EmptyWord_NotAdded)
{
    enchant_dict_add_to_session(_dict, "", -1);
    CHECK(!addToSessionCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_WordSize0_NotAdded)
{
    enchant_dict_add_to_session(_dict, "hello", 0);
    CHECK(!addToSessionCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToSession_TestFixture,
             EnchantDictionaryAddToSession_InvalidUtf8Word_NotAdded)
{
    enchant_dict_add_to_session(_dict, "\xa5\xf1\x08", -1);
    CHECK(!addToSessionCalled);
}

TEST_FIXTURE(EnchantDictionaryAddToSessionNotImplemented_TestFixture,
             EnchantDictionaryAddToSessionNotImplemented_WordAddedToSession)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryAddToSessionNotImplemented_TestFixture,
             EnchantDictionaryAddToSessionNotImplemented_NotPassedOnToProvider)
{
    enchant_dict_add_to_session(_dict, "hello", -1);
    CHECK(!addToSessionCalled);
}
