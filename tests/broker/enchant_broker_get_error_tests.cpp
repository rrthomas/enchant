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
 * enchant_broker_get_error
 * @broker: A non-null broker
 *
 * Returns a const char string or NULL describing the last exception.
 * WARNING: error is transient and is likely cleared as soon as the 
 * next broker operation happens
 */


/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerTestFixture, 
             EnchantBrokerGetError_HasPreviousError_Error)
{
    std::string errorMessage("something bad happened");
    SetErrorOnMockProvider(errorMessage);

    const char * result = enchant_broker_get_error(_broker);
    CHECK(result);
    if(result)
    {
        CHECK_EQUAL(errorMessage.c_str(), result);
    }
}

TEST_FIXTURE(EnchantBrokerTestFixture, 
             EnchantBrokerGetError_NoPreviousError_Null)
{
    CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions

TEST(EnchantBrokerGetError_NullBroker_Null)
{
    CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(NULL));
}
