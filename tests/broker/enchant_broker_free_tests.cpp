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
#include "EnchantBrokerTestFixture.h"

/**
 * enchant_broker_free
 * @broker: A non-null #EnchantBroker
 *
 * Destroys the broker object
 *
 * Free must only be called once!
 */

bool disposeWasCalled;
static void
Dispose (EnchantProvider *me)
{
    disposeWasCalled = true;
    MockProviderDispose(me);
}

static void
AlternativeDispose (EnchantProvider *me)
{
    MockProviderDispose(me);
}

bool disposeDictionaryCalled;
static void
DisposeDictionary (EnchantProvider *me, EnchantDict * dict)
{
    disposeDictionaryCalled = true;
    MockProviderDisposeDictionary(me, dict);
}

static void ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->dispose = Dispose;
     me->request_dict = MockEnGbAndQaaProviderRequestDictionary;
     me->dispose_dict = DisposeDictionary;
}

struct EnchantBrokerFreeTestFixture: EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerFreeTestFixture():EnchantBrokerTestFixture(ProviderConfiguration)
    {
        disposeWasCalled = false;
        disposeDictionaryCalled = false;
    }
};


static void AlternativeDisposeProviderConfiguration (EnchantProvider * me, const char *)
{
     me->dispose = AlternativeDispose;
}

struct EnchantBrokerFreeAlternativeDisposeTestFixture: EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerFreeAlternativeDisposeTestFixture():EnchantBrokerTestFixture(AlternativeDisposeProviderConfiguration)
    {  }
};


/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST(EnchantBrokerFree)
{
    EnchantBroker* broker = enchant_broker_init ();
    enchant_broker_free(broker);
    broker = NULL;
}

TEST_FIXTURE(EnchantBrokerFreeTestFixture,
             EnchantBrokerFree_DisposesProviders)
{
    enchant_broker_free(_broker);
    CHECK(disposeWasCalled);
    _broker = NULL;
}

TEST_FIXTURE(EnchantBrokerFreeAlternativeDisposeTestFixture,
             EnchantBrokerFree_ProviderAlternativeDispose)
{
    enchant_broker_free(_broker);
    _broker = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerFreeTestFixture,
             EnchantBrokerFree_NullBroker_DoNothing)
{
    enchant_broker_free(NULL);
    CHECK(!disposeWasCalled);
}

TEST_FIXTURE(EnchantBrokerFreeTestFixture,
             EnchantBrokerFree_DictionaryNotFreed_FreesDictionary)
{
    EnchantDict* dict = enchant_broker_request_dict(_broker, "en_GB");
    CHECK(dict);
    CHECK(!disposeDictionaryCalled);

    enchant_broker_free(_broker);
    _broker = NULL;
    CHECK(disposeDictionaryCalled);
}
