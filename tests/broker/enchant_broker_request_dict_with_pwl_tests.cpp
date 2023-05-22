/* Copyright (c) 2007 Eric Scott Albright
 * Copyright (c) 2023 Reuben Thomas
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
static EnchantDict * RequestDictionaryWithPwl (EnchantProvider *me, const char *tag)
{
    requestDictionaryCalled = true;
    return MockEnGbAndQaaProviderRequestDictionary(me, tag);
}

static void Request_Dictionary_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = RequestDictionaryWithPwl;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantBrokerRequestDictionaryWithPwl_TestFixture : EnchantBrokerTestFixture
{
    EnchantDict* _pwl;
    std::string _pwlFileName;

    //Setup
    EnchantBrokerRequestDictionaryWithPwl_TestFixture():
            EnchantBrokerTestFixture(Request_Dictionary_ProviderConfiguration)
    {
        _dict = NULL;
        requestDictionaryCalled = false;
        _pwl = RequestPersonalDictionary();
        _pwlFileName = GetLastPersonalDictionaryFileName();
    }

    //Teardown
    ~EnchantBrokerRequestDictionaryWithPwl_TestFixture()
    {
        FreeDictionary(_dict);
        FreeDictionary(_pwl);
    }

    EnchantDict* _dict;
};

/**
 * enchant_broker_request_dict_with_pwl
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 * @pwl: A non-null pathname in the GLib file name encoding (UTF-8 on Windows) to the personal wordlist file
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be found, or if the pwl could not be opened.
 * This dictionary is reference counted.
 */



/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_ProviderHas_CallsProvider)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB", _pwlFileName.c_str());
    CHECK(_dict);
    CHECK(requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_CalledTwice_CallsProviderOnceReturnsSame)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB", _pwlFileName.c_str());
    requestDictionaryCalled = false;
    EnchantDict* dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB", _pwlFileName.c_str());
    CHECK(!requestDictionaryCalled);

    CHECK_EQUAL(_dict, dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_ProviderDoesNotHave_CallsProvider)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en", _pwlFileName.c_str());
    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_ProviderHasBase_CallsProvider)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "qaa_CA", _pwlFileName.c_str());
    CHECK(_dict);
    CHECK(requestDictionaryCalled);
}


TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_WhitespaceSurroundingLanguageTag_Removed)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "\n\r en_GB \t\f", _pwlFileName.c_str());
    CHECK(_dict);
}

/* Vertical tab is not considered to be whitespace in glib!
    See bug# 59388 http://bugzilla.gnome.org/show_bug.cgi?id=59388
*/
TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_VerticalTabBeforeLanguageTag_NotRemoved)
{
  _dict = enchant_broker_request_dict_with_pwl(_broker, "\ven_GB", _pwlFileName.c_str());
  CHECK_EQUAL((void*)NULL, (void*)_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_VerticalTabAfterLanguageTag_NotRemoved)
{
  _dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB\v", _pwlFileName.c_str());
  CHECK_EQUAL((void*)NULL, (void*)_dict);
}


TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_AtSignInLanguageTag_RemovesToTail)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB@euro", _pwlFileName.c_str());
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_PeriodInLanguageTag_RemovesToTail)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en_GB.UTF-8", _pwlFileName.c_str());
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_HyphensInLanguageTag_SubstitutedWithUnderscore)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en-GB", _pwlFileName.c_str());
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_DifferentCase_Finds)
{
  _dict = enchant_broker_request_dict_with_pwl(_broker, "En_gb", _pwlFileName.c_str());
  CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_DifferentCase_NoRegion_Finds)
{
  _dict = enchant_broker_request_dict_with_pwl(_broker, "QAA", _pwlFileName.c_str());
  CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockProvider("something bad happened");

  _dict = enchant_broker_request_dict_with_pwl(_broker, "en-GB", _pwlFileName.c_str());

  CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

// ordering of providers for request is tested by enchant_broker_set_ordering tests

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_NullBroker_NULL)
{
    _dict = enchant_broker_request_dict(NULL, "en_GB");

    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(!requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_NullLanguageTag_NULL)
{
    _dict = enchant_broker_request_dict(_broker, NULL);

    CHECK_EQUAL((void*)NULL, (void*)_dict);
    CHECK(!requestDictionaryCalled);
}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_EmptyLanguageTag_NULL)
{

    _dict = enchant_broker_request_dict_with_pwl(_broker, "", _pwlFileName.c_str());

    CHECK_EQUAL((void*)NULL, _dict);
    CHECK(!requestDictionaryCalled);

}

TEST_FIXTURE(EnchantBrokerRequestDictionaryWithPwl_TestFixture,
             EnchantBrokerRequestDictionaryWithPwl_InvalidTag_NULL_ErrorSet)
{
    _dict = enchant_broker_request_dict_with_pwl(_broker, "en~US", _pwlFileName.c_str());
    CHECK_EQUAL((void*)NULL, _dict);
    CHECK(NULL != enchant_broker_get_error(_broker));
}
