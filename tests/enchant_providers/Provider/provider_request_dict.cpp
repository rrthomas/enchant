/* Copyright (c) 2008 Eric Scott Albright
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

#include "../unittest_enchant_providers.h"

struct ProviderRequestDictionary_TestFixture : Provider_TestFixture
{
    EnchantDict* _dict;
    //Setup
    ProviderRequestDictionary_TestFixture():_dict(NULL)
    { 
    }
    //Teardown
    ~ProviderRequestDictionary_TestFixture()
    {
        if (_dict)
            _provider->dispose_dict(_provider, _dict);
    }
};

// request_dict is optional

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(ProviderRequestDictionary_TestFixture, 
             ProviderRequestDictionary_ProviderDoesNotHave_ReturnsNull)
{
    _dict = (*_provider->request_dict) (_provider, "zxx"); /*zxx is no linguistic content*/
    CHECK_EQUAL((void*)NULL, _dict);
}
