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
/**
 * enchant_broker_free_dict
 * @broker: A non-null #EnchantBroker
 * @dict: A non-null #EnchantDict
 *
 * Releases the dictionary when you are done using it
 * 
 * Must only be called once per dictionary request
 */

EnchantDict* dictionaryToBeFreed=NULL;
static void
DisposeDictionary (EnchantProvider *me, EnchantDict * dict)
{
    dictionaryToBeFreed = dict;
    MockProviderDisposeDictionary(me, dict);
}

static void
AlternativeDisposeDictionary (EnchantProvider *me, EnchantDict * dict)
{
    MockProviderDisposeDictionary(me, dict);
}

static void ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockEnGbAndQaaProviderRequestDictionary;
     me->dispose_dict = DisposeDictionary;
}

struct EnchantBrokerFreeDictTestFixture: EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerFreeDictTestFixture():EnchantBrokerTestFixture(ProviderConfiguration)
    {
        _dictionary = RequestDictionary("en-GB");
        dictionaryToBeFreed=NULL;
    }

    //Teardown
    ~EnchantBrokerFreeDictTestFixture()
    {
        FreeDictionary(_dictionary);
    }

    EnchantDict* _dictionary;
};

static void AlternativeDisposeProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockEnGbAndQaaProviderRequestDictionary;
     me->dispose_dict = AlternativeDisposeDictionary;
}

struct EnchantBrokerFreeDictAlternativeDisposeTestFixture: EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerFreeDictAlternativeDisposeTestFixture():EnchantBrokerTestFixture(AlternativeDisposeProviderConfiguration)
    {
        _dictionary = enchant_broker_request_dict(_broker, "en-GB");
        dictionaryToBeFreed=NULL;
    }

    //Teardown
    ~EnchantBrokerFreeDictAlternativeDisposeTestFixture(){
        FreeDictionary(_dictionary);
    }

    EnchantDict* _dictionary;
};

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict)
{
    enchant_broker_free_dict(_broker, _dictionary);
    CHECK_EQUAL(_dictionary, dictionaryToBeFreed);
    _dictionary = NULL;
}

TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict_HasPreviousError_ErrorCleared)
{
  
    SetErrorOnMockProvider("something bad happened");

    enchant_broker_free_dict(_broker, _dictionary);
    _dictionary = NULL;

    CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict_HasTwoInstances_NoCrash)
{
    EnchantDict* dictionary1 = enchant_broker_request_dict(_broker, "en-GB");
    EnchantDict* dictionary2 = enchant_broker_request_dict(_broker, "en-GB");
    enchant_broker_free_dict(_broker, dictionary1);
    enchant_broker_free_dict(_broker, dictionary2);
}

TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict_CompositeDictionary_NoCrash)
{
    EnchantDict* dictionary = enchant_broker_request_dict(_broker, "en-GB:qaa");
    enchant_broker_free_dict(_broker, dictionary);
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict_NullBroker_DoNothing)
{
    enchant_broker_free_dict(NULL, _dictionary);
    CHECK_EQUAL((EnchantDict*)NULL, dictionaryToBeFreed);
}

TEST_FIXTURE(EnchantBrokerFreeDictTestFixture, 
             EnchantBrokerFreeDict_NullDict_DoNothing)
{
    enchant_broker_free_dict(_broker, NULL);
    CHECK_EQUAL((EnchantDict*)NULL, dictionaryToBeFreed);
}

TEST_FIXTURE(EnchantBrokerFreeDictAlternativeDisposeTestFixture, 
             EnchantBrokerFreeDict_NotOnProvider_DoNothing)
{
    enchant_broker_free_dict(_broker, _dictionary);
    _dictionary = NULL;
}
