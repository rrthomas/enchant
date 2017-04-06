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

#include <UnitTest++.h>
#include <enchant-provider.h>
#include <glib.h>
#include <string.h>

#include "../unittest_enchant_providers.h"

struct ProviderDescribe_TestFixture : Provider_TestFixture
{
    //Setup
    ProviderDescribe_TestFixture()
    { 
    }
    //Teardown
    ~ProviderDescribe_TestFixture()
    {
    }
};

// describe is mandatory

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(ProviderDescribe_TestFixture, 
             ProviderDescribe_FunctionExists)
{
    CHECK(_provider->describe != NULL);
}

TEST_FIXTURE(ProviderDescribe_TestFixture, 
             ProviderDescribe_ReturnNonNull)
{
    CHECK((*_provider->describe)(_provider) != NULL);
}

TEST_FIXTURE(ProviderDescribe_TestFixture, 
             ProviderDescribe_ReturnNotEmpty)
{
    CHECK(strlen((*_provider->describe)(_provider)) != 0);
}

TEST_FIXTURE(ProviderDescribe_TestFixture, 
             ProviderDescribe_ReturnsValidUtf8String)
{
    CHECK(g_utf8_validate((*_provider->describe)(_provider), -1, NULL));
}
	