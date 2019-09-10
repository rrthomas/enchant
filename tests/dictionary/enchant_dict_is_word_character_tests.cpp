/* Copyright (c) 2017 Reuben Thomas
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
#include <vector>
#include <algorithm>

#include "EnchantDictionaryTestFixture.h"

static bool dictIsWordCharacterCalled;

struct EnchantDictionaryIsWordCharacterTestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryIsWordCharacterTestFixtureBase(ConfigureHook userConfiguration):
            EnchantDictionaryTestFixture(userConfiguration)
    { 
        dictIsWordCharacterCalled = false;
    }
};

static int
MyMockDictionaryIsWordCharacter (EnchantDict * dict, uint32_t uc, size_t n)
{
    dictIsWordCharacterCalled = true;
    return 0;
}

static EnchantDict* MockProviderRequestIsWordCharacterMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->is_word_character = MyMockDictionaryIsWordCharacter;
    return dict;
}

static void DictionaryIsWordCharacter_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestIsWordCharacterMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}




struct EnchantDictionaryIsWordCharacter_TestFixture : EnchantDictionaryIsWordCharacterTestFixtureBase
{
    //Setup
    EnchantDictionaryIsWordCharacter_TestFixture():
            EnchantDictionaryIsWordCharacterTestFixtureBase(DictionaryIsWordCharacter_ProviderConfiguration)
    { }
};




static EnchantDict* MockProviderRequestNoIsWordCharacterMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->is_word_character = NULL;
    return dict;
}

static void DictionaryNoIsWordCharacter_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestNoIsWordCharacterMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryIsWordCharacterNotImplemented_TestFixture : EnchantDictionaryIsWordCharacterTestFixtureBase
{
    //Setup
    EnchantDictionaryIsWordCharacterNotImplemented_TestFixture():
            EnchantDictionaryIsWordCharacterTestFixtureBase(DictionaryNoIsWordCharacter_ProviderConfiguration)
    { }
};


/**
 * enchant_dict_is_word_character
 */
/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryIsWordCharacter_TestFixture,
             EnchantDictionaryIsWordCharacter_SuppliedMethodIsCalled)
{
    enchant_dict_is_word_character(_dict, 'a', 0);
    CHECK(dictIsWordCharacterCalled);
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_NoSuppliedMethod)
{
    enchant_dict_is_word_character(_dict, 'a', 0);
    CHECK(!dictIsWordCharacterCalled);
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_NullDictUsesBuiltInMethod)
{
    CHECK(enchant_dict_is_word_character(NULL, 'a', 0));
    CHECK(!dictIsWordCharacterCalled);
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_LetterAcceptedAtStart)
{
    CHECK(enchant_dict_is_word_character(_dict, 'a', 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_LetterAcceptedInMiddle)
{
    CHECK(enchant_dict_is_word_character(_dict, 'a', 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_LetterAcceptedAtEnd)
{
    CHECK(enchant_dict_is_word_character(_dict, 'a', 2));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_MultibyteLetterAcceptedAtStart)
{
    CHECK(enchant_dict_is_word_character(_dict, 0xe0, 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_MultibyteLetterAcceptedInMiddle)
{
    CHECK(enchant_dict_is_word_character(_dict, 0xe0, 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_MultibyteLetterAcceptedAtEnd)
{
    CHECK(enchant_dict_is_word_character(_dict, 0xe0, 2));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_DigitAcceptedAtStart)
{
    CHECK(enchant_dict_is_word_character(_dict, '0', 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_DigitAcceptedInMiddle)
{
    CHECK(enchant_dict_is_word_character(_dict, '0', 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_DigitAcceptedAtEnd)
{
    CHECK(enchant_dict_is_word_character(_dict, '0', 2));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_InvalidNotAcceptedAtStart)
{
    CHECK(!enchant_dict_is_word_character(_dict, '!', 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_InvalidNotAcceptedInMiddle)
{
    CHECK(!enchant_dict_is_word_character(_dict, '!', 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_InvalidNotAcceptedAtEnd)
{
    CHECK(!enchant_dict_is_word_character(_dict, '!', 2));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_QuoteAcceptedAtStart)
{
    CHECK(enchant_dict_is_word_character(_dict, '\'', 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_QuoteAcceptedInMiddle)
{
    CHECK(enchant_dict_is_word_character(_dict, '\'', 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_QuoteNotAcceptedAtEnd)
{
    CHECK(!enchant_dict_is_word_character(_dict, '\'', 2));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_HyphenNotAcceptedAtStart)
{
    CHECK(!enchant_dict_is_word_character(_dict, '-', 0));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_HyphenAcceptedInMiddle)
{
    CHECK(enchant_dict_is_word_character(_dict, '-', 1));
}

TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_HyphenNotAcceptedAtEnd)
{
    CHECK(!enchant_dict_is_word_character(_dict, '-', 2));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryIsWordCharacterNotImplemented_TestFixture,
             EnchantDictionaryIsWordCharacter_LetterNotAcceptedInInvalidPosition)
{
    CHECK(!enchant_dict_is_word_character(_dict, 'a', 3));
}
