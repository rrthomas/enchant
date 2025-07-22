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

struct EnchantBrokerRequestPwlDictionary_TestFixture : EnchantBrokerTestFixture
{
    //Setup
    EnchantBrokerRequestPwlDictionary_TestFixture()
    { 
        _dict = NULL;
        _pwlFile = GetTemporaryFilename("epwl");
    }

    //Teardown
    ~EnchantBrokerRequestPwlDictionary_TestFixture()
    {
        FreeDictionary(_dict);
        DeleteFile(_pwlFile);
    }

    EnchantDict* _dict;
    std::string _pwlFile;
};

/**
 * enchant_broker_request_pwl_dict
 *
 * PWL is a personal wordlist file, 1 entry per line
 *
 * Returns: 
 */


/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture, 
             EnchantBrokerRequestPwlDictionary_FileExists)
{
    CreateFile(_pwlFile);
    _dict = enchant_broker_request_pwl_dict(_broker, _pwlFile.c_str());
    CHECK(_dict);
}

TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture, 
             EnchantBrokerRequestPwlDictionary_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockProvider("something bad happened");

  _dict = enchant_broker_request_pwl_dict(_broker, _pwlFile.c_str());

  CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}



/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture,
             EnchantBrokerRequestPwlDictionary_NullBroker_NULL)
{
    _dict = enchant_broker_request_pwl_dict(NULL, _pwlFile.c_str());

    CHECK_EQUAL((void*)NULL, (void*)_dict);
}

TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture,
             EnchantBrokerRequestPwlDictionary_NullFilename_NULL)
{
    _dict = enchant_broker_request_pwl_dict(_broker, NULL);

    CHECK_EQUAL((void*)NULL, (void*)_dict);
}

TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture,
             EnchantBrokerRequestPwlDictionary_EmptyFilename_NULL)
{
    _dict = enchant_broker_request_pwl_dict(_broker, "");

    CHECK_EQUAL((void*)NULL, _dict);
}

#if defined(_WIN32)
// Colon is illegal for Windows but okay for Linux and macOS;
TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture,
             EnchantBrokerRequestPwlDictionary_IllegalFilename_NULL)
{
    _dict = enchant_broker_request_pwl_dict(_broker, ":");
    CHECK(!_dict);
    CHECK((void*)enchant_broker_get_error(_broker));
}

TEST_FIXTURE(EnchantBrokerRequestPwlDictionary_TestFixture,
             EnchantBrokerRequestPwlDictionary_IllegalUtf8InFilename_NULL)
{
    _dict = enchant_broker_request_pwl_dict(_broker, "abc\xa5\xf1\x08");
    CHECK(!_dict);
}
#endif
