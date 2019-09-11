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

#include "EnchantDictionaryTestFixture.h"

static bool dictGetExtraWordCharactersCalled;

struct EnchantDictionaryGetExtraWordCharactersTestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryGetExtraWordCharactersTestFixtureBase(ConfigureHook userConfiguration):
            EnchantDictionaryTestFixture(userConfiguration)
    { 
        dictGetExtraWordCharactersCalled = false;
        _extraWordCharacters = NULL;
    }

    const char* _extraWordCharacters;
};

static const char *
MyMockDictionaryGetExtraWordCharacters (EnchantDict * dict)
{
    dictGetExtraWordCharactersCalled = true;
    return "01-";
}

static EnchantDict* MockProviderRequestGetExtraWordCharactersMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->get_extra_word_characters = MyMockDictionaryGetExtraWordCharacters;
    return dict;
}

static void DictionaryGetExtraWordCharacters_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestGetExtraWordCharactersMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}




struct EnchantDictionaryGetExtraWordCharacters_TestFixture : EnchantDictionaryGetExtraWordCharactersTestFixtureBase
{
    //Setup
    EnchantDictionaryGetExtraWordCharacters_TestFixture():
            EnchantDictionaryGetExtraWordCharactersTestFixtureBase(DictionaryGetExtraWordCharacters_ProviderConfiguration)
    { }
};




static EnchantDict* MockProviderRequestNoGetExtraWordCharactersMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->get_extra_word_characters = NULL;
    return dict;
}

static void DictionaryNoGetExtraWordCharacters_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestNoGetExtraWordCharactersMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryGetExtraWordCharactersNotImplemented_TestFixture : EnchantDictionaryGetExtraWordCharactersTestFixtureBase
{
    //Setup
    EnchantDictionaryGetExtraWordCharactersNotImplemented_TestFixture():
            EnchantDictionaryGetExtraWordCharactersTestFixtureBase(DictionaryNoGetExtraWordCharacters_ProviderConfiguration)
    { }
};


/**
 * enchant_dict_get_extra_word_characters
 */
/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryGetExtraWordCharacters_TestFixture,
             EnchantDictionaryGetExtraWordCharacters_SuppliedMethodCalled)
{
    _extraWordCharacters = enchant_dict_get_extra_word_characters(_dict);
    CHECK(dictGetExtraWordCharactersCalled);
}

TEST_FIXTURE(EnchantDictionaryGetExtraWordCharacters_TestFixture,
             EnchantDictionaryGetExtraWordCharacters_SuppliedMethodExpectedResult)
{
    _extraWordCharacters = enchant_dict_get_extra_word_characters(_dict);
    CHECK_EQUAL(_extraWordCharacters, "01-");
}

TEST_FIXTURE(EnchantDictionaryGetExtraWordCharactersNotImplemented_TestFixture,
             EnchantDictionaryGetExtraWordCharacters_NoSuppliedMethod)
{
    _extraWordCharacters = enchant_dict_get_extra_word_characters(_dict);
    CHECK(!dictGetExtraWordCharactersCalled);
}

TEST_FIXTURE(EnchantDictionaryGetExtraWordCharactersNotImplemented_TestFixture,
             EnchantDictionaryGetExtraWordCharacters_NoSuppliedMethodExpectedResult)
{
    _extraWordCharacters = enchant_dict_get_extra_word_characters(_dict);
    CHECK_EQUAL(_extraWordCharacters, "");
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryGetExtraWordCharacters_TestFixture,
             EnchantDictionaryGetExtraWordCharacters_NullDictionarySuppliedMethodNotCalled)
{
    _extraWordCharacters = enchant_dict_get_extra_word_characters(NULL);
    CHECK(!dictGetExtraWordCharactersCalled);
}
