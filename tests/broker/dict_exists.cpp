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

static int dictionaryExistsCalled;
static int DoesDictionaryExist (EnchantProvider * me, const char *const tag)
{
    dictionaryExistsCalled++;
    return MockEnGbAndQaaProviderDictionaryExists(me, tag);
}

static void Dictionary_Exists_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->dictionary_exists=DoesDictionaryExist;
}

struct EnchantBrokerDictExists_ProviderImplementsDictionaryExist_TestFixture : EnchantBrokerTestFixture{
  //Setup
    EnchantBrokerDictExists_ProviderImplementsDictionaryExist_TestFixture():
        EnchantBrokerTestFixture(Dictionary_Exists_ProviderConfiguration)
    { dictionaryExistsCalled=0; }
};

#define EnchantBrokerDictExistsTestFixture EnchantBrokerDictExists_ProviderImplementsDictionaryExist_TestFixture
#define DictionaryExistsMethodCalled dictionaryExistsCalled
#include "dict_exists.i"

static int listDictionariesCalled;
static char** ListDictionaries (EnchantProvider * me, size_t * out_n_dicts)
{
   listDictionariesCalled++;
    
   return MockEnGbAndQaaProviderListDictionaries(me, out_n_dicts);
}

static void List_Dictionaries_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->list_dicts=ListDictionaries;
}

struct EnchantBrokerDictExists_ProviderImplementsListDictionaries_TestFixture : EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerDictExists_ProviderImplementsListDictionaries_TestFixture():
            EnchantBrokerTestFixture(List_Dictionaries_ProviderConfiguration)
    { 
        listDictionariesCalled = 0;
    }

};

#undef EnchantBrokerDictExistsTestFixture
#define EnchantBrokerDictExistsTestFixture EnchantBrokerDictExists_ProviderImplementsListDictionaries_TestFixture
#undef DictionaryExistsMethodCalled
#define DictionaryExistsMethodCalled listDictionariesCalled
#include "dict_exists.i"

static void ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockEnGbAndQaaProviderRequestDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
     me->list_dicts = ListDictionaries;
     me->dictionary_exists = DoesDictionaryExist;
}

struct EnchantBrokerDictExists_ProviderImplementsAll_TestFixture : EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerDictExists_ProviderImplementsAll_TestFixture():
            EnchantBrokerTestFixture(ProviderConfiguration)
    { 
      listDictionariesCalled = 0;
      dictionaryExistsCalled = 0;
    }
};

TEST_FIXTURE(EnchantBrokerTestFixture, 
             EnchantBrokerDictExists_ProviderImplementsNoMethods_0)
{
  CHECK_EQUAL(0, enchant_broker_dict_exists(_broker, "en_GB"));
}
