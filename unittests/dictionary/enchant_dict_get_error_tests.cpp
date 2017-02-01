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

/**
 * enchant_dict_get_error
 * @dict: A non-null dictionary
 *
 * Returns a const char string or NULL describing the last exception.
 * WARNING: error is transient. It will likely be cleared as soon as 
 * the next dictionary operation is called
 *
 * Returns: an error message
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantDictionaryTestFixture, 
             EnchantDictionaryGetError_HasPreviousError_Error)
{
    std::string errorMessage("something bad happened");
    SetErrorOnMockDictionary(errorMessage);

    CHECK_EQUAL(errorMessage.c_str(), enchant_dict_get_error(_dict));
}

TEST_FIXTURE(EnchantDictionaryTestFixture, 
             EnchantDictionaryGetErrorOnPwl_HasPreviousError_Error)
{
    std::string errorMessage("something bad happened");
    enchant_dict_set_error(_pwl, errorMessage.c_str());


    CHECK_EQUAL(errorMessage.c_str(), enchant_dict_get_error(_pwl));
}

TEST_FIXTURE(EnchantDictionaryTestFixture, 
             EnchantDictionaryGetError_NoPreviousError_Null)
{
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST(EnchantDictionaryGetError_NullBroker_Null)
{
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(NULL));
}
