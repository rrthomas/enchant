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
#include "EnchantBrokerTestFixture.h"
#include <vector>


void EnchantDictionaryDescribeCallback (const char * const lang_tag,
				                          const char * const provider_name,
				                          const char * const provider_desc,
				                          const char * const provider_file,
				                          void * user_data)
{
    std::vector<DictionaryDescription>* dictionaryList = reinterpret_cast<std::vector<DictionaryDescription>*>(user_data);
    dictionaryList->push_back(DictionaryDescription(lang_tag, provider_name, provider_desc, provider_file));
}

static void * global_user_data;
void EnchantDictionaryDescribeAssignUserDataToStaticCallback (const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              void * user_data)
{
  global_user_data = user_data;
}

static bool listDictionariesCalled;
static char** ListDictionaries (EnchantProvider * me, size_t * out_n_dicts)
{
   listDictionariesCalled = true;
    
   return MockEnGbProviderListDictionaries(me, out_n_dicts);
}

struct EnchantBrokerListDictionaries_TestFixtureBase : EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerListDictionaries_TestFixtureBase(ConfigureHook userConfiguration):
            EnchantBrokerTestFixture(userConfiguration)
    {
        listDictionariesCalled=false; 
        global_user_data = NULL;
    }
    std::vector<DictionaryDescription> _dictionaryList;
};

static void List_Dictionaries_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->list_dicts=ListDictionaries;
}


struct EnchantBrokerListDictionaries_TestFixture : EnchantBrokerListDictionaries_TestFixtureBase
{
    //Setup
    EnchantBrokerListDictionaries_TestFixture():
            EnchantBrokerListDictionaries_TestFixtureBase(List_Dictionaries_ProviderConfiguration)
    { }
};


static char ** 
DuplicateEnGbListDictionaries (EnchantProvider *, 
		 	                  size_t * out_n_dicts)
{
    *out_n_dicts = 2;
    char** out_list = g_new0 (char *, *out_n_dicts + 1);
    out_list[0] = g_strdup ("en_GB");
    out_list[1] = g_strdup ("en_GB");

    return out_list;
}

static void List_Dictionaries_ProviderConfigurationDuplicateTags (EnchantProvider * me, const char *)
{
     me->list_dicts=DuplicateEnGbListDictionaries;
}

struct EnchantBrokerListDictionaries_ProviderDuplicateTags_TestFixture : EnchantBrokerListDictionaries_TestFixtureBase
{
    //Setup
    EnchantBrokerListDictionaries_ProviderDuplicateTags_TestFixture():
            EnchantBrokerListDictionaries_TestFixtureBase(List_Dictionaries_ProviderConfigurationDuplicateTags)
    { }
};

/**
 * enchant_broker_list_dicts
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the dictionaries available from
 * all Enchant providers.
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_UserDataNotNull_PassedThrough)
{
    char* userData = const_cast<char*>("some user data");
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeAssignUserDataToStaticCallback, userData);
    CHECK_EQUAL(userData, global_user_data);
}

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_UserDataNull_PassedThrough)
{
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeAssignUserDataToStaticCallback, NULL);
    CHECK_EQUAL((void*)NULL, global_user_data);
}

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_listDictionariesCalledOnProvider)
{
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeCallback, &_dictionaryList);
    CHECK(listDictionariesCalled);
}

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_dictionaryListHasDictionariesFromProvider)
{
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeCallback, &_dictionaryList);
    CHECK_EQUAL((unsigned int)1, _dictionaryList.size());
    if(_dictionaryList.size()>0){
        CHECK(_dictionaryList[0].DataIsComplete());
        CHECK_EQUAL(std::string("en_GB"), _dictionaryList[0].LanguageTag);
        CHECK_EQUAL(std::string("mock"), _dictionaryList[0].Name);
        CHECK_EQUAL(std::string("Mock Provider"), _dictionaryList[0].Description);
    }
}

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture, 
             EnchantBrokerListDictionaries_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockProvider("something bad happened");

    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeCallback, &_dictionaryList);

    CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_NullBroker_DoNotCallDescribeFunction)
{
    enchant_broker_list_dicts(NULL, EnchantDictionaryDescribeAssignUserDataToStaticCallback, NULL);
    CHECK(!listDictionariesCalled);
}

TEST_FIXTURE(EnchantBrokerListDictionaries_TestFixture,
             EnchantBrokerListDictionaries_NullCallback_DoNotCallDescribeFunction)
{
    enchant_broker_list_dicts(_broker, NULL, NULL);
    CHECK(!listDictionariesCalled);
}

TEST_FIXTURE(EnchantBrokerTestFixture,
             EnchantBrokerListDictionaries_ProviderLacksListDictionaries_CallbackNeverCalled)
{
    void* userData = const_cast<char*>("some user data");
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeAssignUserDataToStaticCallback, userData);
    CHECK_EQUAL((void*)NULL, global_user_data);
}

TEST_FIXTURE(EnchantBrokerListDictionaries_ProviderDuplicateTags_TestFixture,
             EnchantBrokerListDictionaries_ProviderDuplicateTags_CallbackCalledOnlyOncePerUniqueTag)
{
    enchant_broker_list_dicts(_broker, EnchantDictionaryDescribeCallback, &_dictionaryList);
    CHECK_EQUAL((unsigned int)1, _dictionaryList.size());
}
