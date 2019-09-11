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
#include <glib.h>
#include <string.h>

#include "../unittest_enchant_providers.h"

struct ProviderIdentify_TestFixture : Provider_TestFixture
{
    //Setup
    ProviderIdentify_TestFixture()
    { 
    }
    //Teardown
    ~ProviderIdentify_TestFixture()
    {
    }
};

// identify is mandatory

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(ProviderIdentify_TestFixture, 
             ProviderIdentify_FunctionExists)
{
    CHECK(_provider->identify != NULL);
}

TEST_FIXTURE(ProviderIdentify_TestFixture, 
             ProviderIdentify_ReturnNonNull)
{
    CHECK((*_provider->identify)(_provider) != NULL);
}

TEST_FIXTURE(ProviderIdentify_TestFixture, 
             ProviderIdentify_ReturnNotEmpty)
{
    CHECK(strlen((*_provider->identify)(_provider)) != 0);
}

TEST_FIXTURE(ProviderIdentify_TestFixture, 
             ProviderIdentify_ReturnsValidUtf8String)
{
    CHECK(g_utf8_validate((*_provider->identify)(_provider), -1, NULL));
}
	