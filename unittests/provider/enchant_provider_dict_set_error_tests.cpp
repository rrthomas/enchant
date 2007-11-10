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
#include <enchant-provider.h>
#include "../EnchantDictionaryTestFixture.h"

struct EnchantDictionarySetErrorTests : EnchantDictionaryTestFixture
{
  //Setup
  std::string GetErrorMessage(){
      char* error = enchant_dict_get_error(_dict);
      if(error == NULL){
          error = "";
      }
      return std::string(error);
  }
};

/**
 * enchant_dict_set_error
 * @dict: A non-null dictionary
 * @err: A non-null error message
 *
 * Sets the current runtime error to @err. This API is private to the
 * providers.
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantDictionarySetErrorTests, 
             SetErrorMessageOnDictionary)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_dict_set_error(_dict, expectedErrorMessage.c_str());

    CHECK_EQUAL(expectedErrorMessage, GetErrorMessage());
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionarySetErrorTests, 
             SetErrorMessageOnDictionary_NullDictionary_NoErrorSet)
{
    enchant_dict_set_error(NULL, "Error message to display");

    CHECK_EQUAL(std::string(), GetErrorMessage());
}

TEST_FIXTURE(EnchantDictionarySetErrorTests, 
             SetErrorMessageOnDictionary_NullError_NoErrorSet)
{
    enchant_dict_set_error(_dict, NULL);

    CHECK_EQUAL(std::string(), GetErrorMessage());
}


TEST_FIXTURE(EnchantDictionarySetErrorTests, 
             SetErrorMessageOnDictionary_MessageCopied)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_dict_set_error(_dict, expectedErrorMessage.c_str());

    expectedErrorMessage[0] = 'e';
    CHECK(expectedErrorMessage != GetErrorMessage());
}