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

static bool requestDictionaryCalled;
static EnchantDict * RequestDictionary (EnchantProvider *me, const char *tag)
{
    requestDictionaryCalled = true;
    return MockEnGbAndQaaProviderRequestDictionary(me, tag);
}

static void Request_Dictionary_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = RequestDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantBrokerRequestDictionary_TestFixture : EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerRequestDictionary_TestFixture():
            EnchantBrokerTestFixture(Request_Dictionary_ProviderConfiguration)
    { 
        _dict = NULL;
        requestDictionaryCalled = false;
    }

    //Teardown
    ~EnchantBrokerRequestDictionary_TestFixture()
    {
        FreeDictionary(_dict);
    }

    EnchantDict* _dict;
};

/**
 * enchant_broker_request_dict
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be found.
 */



/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_ProviderHas_CallsProvider)
{
    _dict = enchant_broker_request_dict(_broker, "en_GB");
    CHECK(_dict);
    CHECK(requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_CalledTwice_CallsProviderOnceReturnsSame)
{
    _dict = enchant_broker_request_dict(_broker, "en_GB");
    requestDictionaryCalled = false;
    EnchantDict* dict = enchant_broker_request_dict(_broker, "en_GB");
    CHECK(!requestDictionaryCalled);

    CHECK_EQUAL(_dict, dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_ProviderDoesNotHave_CallsProvider)
{
    _dict = enchant_broker_request_dict(_broker, "en");
    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_ProviderHasBase_CallsProvider)
{
    _dict = enchant_broker_request_dict(_broker, "qaa_CA");
    CHECK(_dict);
    CHECK(requestDictionaryCalled);
}


TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_WhitespaceSurroundingLanguageTag_Removed)
{
    _dict = enchant_broker_request_dict(_broker, "\n\r en_GB \t\f");
    CHECK(_dict);
}

/* Vertical tab is not considered to be whitespace in glib!
    See bug# 59388 http://bugzilla.gnome.org/show_bug.cgi?id=59388
*/
TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_VerticalTabBeforeLanguageTag_NotRemoved)
{
  _dict = enchant_broker_request_dict(_broker, "\ven_GB");
  CHECK_EQUAL((void*)NULL, (void*)_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_VerticalTabAfterLanguageTag_NotRemoved)
{
  _dict = enchant_broker_request_dict(_broker, "en_GB\v");
  CHECK_EQUAL((void*)NULL, (void*)_dict);
}


TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_AtSignInLanguageTag_RemovesToTail)
{
    _dict = enchant_broker_request_dict(_broker, "en_GB@euro");
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_PeriodInLanguageTag_RemovesToTail)
{
    _dict = enchant_broker_request_dict(_broker, "en_GB.UTF-8");
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_HyphensInLanguageTag_SubstitutedWithUnderscore)
{
    _dict = enchant_broker_request_dict(_broker, "en-GB");
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_DifferentCase_Finds)
{
  _dict = enchant_broker_request_dict(_broker, "En_gb");
  CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_DifferentCase_NoRegion_Finds)
{
  _dict = enchant_broker_request_dict(_broker, "QAA");
  CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockProvider("something bad happened");

  _dict = enchant_broker_request_dict(_broker, "en-GB");

  CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

// ordering of providers for request is tested by enchant_broker_set_ordering tests

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture,
             EnchantBrokerRequestDictionary_NullBroker_NULL)
{
    _dict = enchant_broker_request_dict(NULL, "en_GB");

    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(!requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture,
             EnchantBrokerRequestDictionary_NullLanguageTag_NULL)
{
    _dict = enchant_broker_request_dict(_broker, NULL);

    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(!requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture,
             EnchantBrokerRequestDictionary_EmptyLanguageTag_NULL)
{
    _dict = enchant_broker_request_dict(_broker, "");

    CHECK_EQUAL((void*)NULL, _dict);
    CHECK(!requestDictionaryCalled);

}

TEST_FIXTURE(EnchantBrokerRequestDictionary_TestFixture, 
             EnchantBrokerRequestDictionary_InvalidTag_NULL_ErrorSet)
{
    _dict = enchant_broker_request_dict(_broker, "en~US");
    CHECK_EQUAL((void*)NULL, _dict);
    CHECK(NULL != enchant_broker_get_error(_broker));
}

TEST_FIXTURE(EnchantBrokerTestFixture,
             EnchantBrokerRequestDictionary_ProviderLacksListDictionaries_CallbackNeverCalled)
{
    requestDictionaryCalled = false;
    EnchantDict* dict = enchant_broker_request_dict(_broker, "en_GB");

    CHECK_EQUAL((void*)NULL, dict);
    CHECK(!requestDictionaryCalled);
}
