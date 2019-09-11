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

struct EnchantDictionaryStoreReplacement_TestFixture : EnchantDictionaryTestFixture
{
    static std::string misspelling;
    static std::string correction;

    static bool storeReplacementCalled;

    //Setup
    EnchantDictionaryStoreReplacement_TestFixture():
            EnchantDictionaryTestFixture(DictionaryStoreReplacement_ProviderConfiguration)
    { 
        storeReplacementCalled = false;
        misspelling.clear();
        correction.clear();
    }

    static void
    MockDictionaryStoreReplacement (EnchantDict *,
				       const char *const mis, size_t mis_len,
				       const char *const cor, size_t cor_len)
    {
        misspelling = std::string(mis, mis_len);
        correction = std::string(cor, cor_len);
        storeReplacementCalled = true;
    }

    static EnchantDict*
    MockProviderRequestStoreReplacementMockDictionary(EnchantProvider * me, const char *tag)
    {
        
        EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
        dict->store_replacement = MockDictionaryStoreReplacement;
        return dict;
    }

    static void DictionaryStoreReplacement_ProviderConfiguration (EnchantProvider * me, const char *)
    {
         me->request_dict = MockProviderRequestStoreReplacementMockDictionary;
         me->dispose_dict = MockProviderDisposeDictionary;
    }
};
bool EnchantDictionaryStoreReplacement_TestFixture::storeReplacementCalled;
std::string EnchantDictionaryStoreReplacement_TestFixture::misspelling;
std::string EnchantDictionaryStoreReplacement_TestFixture::correction;

struct EnchantDictionaryLacksStoreReplacement_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryLacksStoreReplacement_TestFixture():
            EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration)
    { }
};




/**
 * enchant_dict_store_replacement
 * @dict: A non-null #EnchantDict
 * @mis: The non-null word you wish to add a correction for, in UTF-8 encoding
 * @mis_len: The byte length of @mis, or -1 for strlen (@mis)
 * @cor: The non-null correction word, in UTF-8 encoding
 * @cor_len: The byte length of @cor, or -1 for strlen (@cor)
 *
 * Notes that you replaced @mis with @cor, so it's possibly more likely
 * that future occurrences of @mis will be replaced with @cor. So it might
 * bump @cor up in the suggestion list.
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ExplicitWordLength)
{
    std::string misspelling("helo");
    std::string correction("hello");
    enchant_dict_store_replacement(_dict, 
                                   misspelling.c_str(), 
                                   misspelling.size(), 
                                   correction.c_str(),
                                   correction.size());
    CHECK(storeReplacementCalled);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::misspelling, misspelling);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::correction, correction);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ComputedWordLength)
{
    std::string misspelling("helo");
    std::string correction("hello");
    enchant_dict_store_replacement(_dict, 
                                   misspelling.c_str(), 
                                   -1, 
                                   correction.c_str(),
                                   -1);
    CHECK(storeReplacementCalled);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::misspelling, misspelling);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::correction, correction);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_OnBrokerPwl)
{
    std::string misspelling("helo");
    std::string correction("hello");
    enchant_dict_store_replacement(_pwl, 
                                   misspelling.c_str(), 
                                   -1, 
                                   correction.c_str(),
                                   -1);
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_NullDict_DoNothing)
{
    enchant_dict_store_replacement(NULL, "helo", -1, "hello", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_NullMisspelling_DoNothing)
{
    enchant_dict_store_replacement(_dict, NULL, -1, "hello", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_NullCorrection_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", -1, NULL, -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_EmptyMisspelling_DoNothing)
{
    enchant_dict_store_replacement(_dict, "", -1, "hello", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_EmptyCorrection_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", -1, "", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ZeroMisspellingLength_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", 0, "hello", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ZeroCorrectionLength_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", -1, "hello", 0);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacement_InvalidUtf8Misspelling_DoNothing)
{
    enchant_dict_store_replacement(_dict, "\xa5\xf1\x08", -1, "hello", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacement_InvalidUtf8Correction_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", -1, "\xa5\xf1\x08", -1);
    CHECK(!storeReplacementCalled);
}

TEST_FIXTURE(EnchantDictionaryLacksStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ProviderLacksStoreReplacement_DoNothing)
{
    enchant_dict_store_replacement(_dict, "helo", -1, "hello", -1);
}

TEST_FIXTURE(EnchantDictionaryStoreReplacement_TestFixture,
             EnchantDictStoreReplacment_ExplicitWordLengthDoesNotCoincideWithNulTerminator)
{
    std::string misspelling("helo1");
    std::string correction("hello1");
    enchant_dict_store_replacement(_dict, 
                                   misspelling.c_str(), 
                                   misspelling.size()-1, 
                                   correction.c_str(),
                                   correction.size()-1);

    misspelling.resize(misspelling.size()-1);
    correction.resize(correction.size()-1);

    CHECK(storeReplacementCalled);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::misspelling, misspelling);
    CHECK_EQUAL(EnchantDictionaryStoreReplacement_TestFixture::correction, correction);
}
