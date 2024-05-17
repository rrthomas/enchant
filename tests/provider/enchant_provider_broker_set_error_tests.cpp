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
#include <enchant-provider.h>

#include "EnchantBrokerTestFixture.h"

struct EnchantBrokerSetErrorTests : EnchantBrokerTestFixture
{
  //Setup
    EnchantBrokerSetErrorTests()
  {
      provider = GetMockProvider();
  }

  std::string GetErrorMessage(){
      const char* error = enchant_broker_get_error(_broker);
      if(error == NULL){
          return std::string();
      }
      return std::string(error);
  }

  EnchantProvider* provider;
};

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerSetErrorTests, 
             SetErrorMessageOnProvider)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_provider_set_error(provider, expectedErrorMessage.c_str());

    CHECK_EQUAL(expectedErrorMessage, GetErrorMessage());
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerSetErrorTests, 
             SetErrorMessageOnProvider_NullProvider_NoErrorSet)
{
    enchant_provider_set_error(NULL, "Error message to display");

    CHECK_EQUAL(std::string(), GetErrorMessage());
}

TEST_FIXTURE(EnchantBrokerSetErrorTests, 
             SetErrorMessageOnProvider_NullError_NoErrorSet)
{
    enchant_provider_set_error(provider, NULL);

    CHECK_EQUAL(std::string(), GetErrorMessage());
}

TEST_FIXTURE(EnchantBrokerSetErrorTests, 
             SetErrorMessageOnProvider_MessageCopied)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_provider_set_error(provider, expectedErrorMessage.c_str());

    expectedErrorMessage[0] = 'e';
    CHECK(expectedErrorMessage != GetErrorMessage());
}
