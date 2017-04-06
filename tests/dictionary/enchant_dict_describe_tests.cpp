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
#include "EnchantDictionaryTestFixture.h"

static bool callbackCalled;

void EnchantSingleDictionaryDescribeCallback (const char * const lang_tag,
				                          const char * const provider_name,
				                          const char * const provider_desc,
				                          const char * const provider_file,
				                          void * user_data)
{
    DictionaryDescription* description = reinterpret_cast<DictionaryDescription*>(user_data);
    *description = DictionaryDescription(lang_tag, provider_name, provider_desc, provider_file);
    callbackCalled = true;
}

static void * global_user_data;
void EnchantSingleDictionaryDescribeAssignUserDataToStaticCallback (const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              void * user_data)
{
    global_user_data = user_data;
    callbackCalled = true;
}

struct EnchantDictionaryDescribe_TestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryDescribe_TestFixtureBase(ConfigureHook userConfig):
            EnchantDictionaryTestFixture(userConfig)
    { 
        callbackCalled = false;
    }
    DictionaryDescription _description;
};

struct EnchantDictionaryDescribe_TestFixture : EnchantDictionaryDescribe_TestFixtureBase
{
    //Setup
    EnchantDictionaryDescribe_TestFixture():
            EnchantDictionaryDescribe_TestFixtureBase(EmptyDictionary_ProviderConfiguration)
    { }
};


/**
 * enchant_dict_describe
 * @broker: A non-null #EnchantDict
 * @dict: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Describes an individual dictionary
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe)
{
    enchant_dict_describe(_dict, EnchantSingleDictionaryDescribeCallback, &_description);
    CHECK(callbackCalled);
    CHECK(_description.DataIsComplete());
    CHECK_EQUAL(std::string("qaa"), _description.LanguageTag);
}

TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe_Pwl)
{
    enchant_dict_describe(_pwl, EnchantSingleDictionaryDescribeCallback, &_description);
    CHECK(callbackCalled);
    CHECK(_description.DataIsComplete());
    CHECK_EQUAL(std::string("Personal Wordlist"), _description.LanguageTag);
    CHECK_EQUAL(std::string("Personal Wordlist"), _description.Name);
    CHECK_EQUAL(std::string("Personal Wordlist"), _description.Description);
}

TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe_NullUserdata_PassedToCallback)
{
    enchant_dict_describe(_dict, EnchantSingleDictionaryDescribeAssignUserDataToStaticCallback, NULL);
    CHECK(callbackCalled);
    CHECK_EQUAL((void*)NULL, global_user_data);
}

TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe_NonNullUserdata_PassedToCallback)
{
    const char* userData = "some user data";
    enchant_dict_describe(_dict, EnchantSingleDictionaryDescribeAssignUserDataToStaticCallback, (void*)userData);
    CHECK(callbackCalled);
    CHECK_EQUAL(userData, global_user_data);
}

TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture, 
             EnchantDictionaryDescribe_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_describe(_dict, EnchantSingleDictionaryDescribeAssignUserDataToStaticCallback, NULL);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}
/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe_NullDict_DoNothing)
{
    enchant_dict_describe(NULL, EnchantSingleDictionaryDescribeCallback, &_description);
    CHECK(!callbackCalled);
}

TEST_FIXTURE(EnchantDictionaryDescribe_TestFixture,
             EnchantDictionaryDescribe_NullCallback_DoNothing)
{
    enchant_dict_describe(_dict, NULL, &_description);
    CHECK(!callbackCalled);
}
