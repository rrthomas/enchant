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

struct ProviderListDicts_TestFixture : Provider_TestFixture
{
	char ** _dicts;

    //Setup
    ProviderListDicts_TestFixture():_dicts(NULL)
    { 
    }
    //Teardown
    ~ProviderListDicts_TestFixture()
    {
        g_strfreev (_dicts);
    }
};

// list_dicts is optional

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(ProviderListDicts_TestFixture, 
             ProviderListDicts_ListReturnedHasContent)
{
    if(_provider->list_dicts)
    {
		size_t n_dicts;

		_dicts = (*_provider->list_dicts) (_provider, &n_dicts);
		for (size_t i = 0; i < n_dicts; i++)
		{
			CHECK(_dicts[i] != NULL);
                }
    }
}

TEST_FIXTURE(ProviderListDicts_TestFixture, 
             ProviderListDicts_ForEachReturned_RequestDictSucceeds)
{
    if(_provider->list_dicts && _provider->request_dict)
    {
		size_t n_dicts;

		_dicts = (*_provider->list_dicts) (_provider, &n_dicts);
		for (size_t i = 0; i < n_dicts; i++)
		{
   			EnchantDict* dict = (*_provider->request_dict) (_provider, _dicts[i]);
			CHECK(dict != NULL);
	        if (dict && _provider->dispose_dict)
		    {
                _provider->dispose_dict(_provider, dict);
		    }
        }
    }
}

TEST_FIXTURE(ProviderListDicts_TestFixture, 
             ProviderListDicts_ForEachReturned_DictExistsSucceeds)
{
    if(_provider->list_dicts && _provider->dictionary_exists)
    {
		size_t n_dicts;

		_dicts = (*_provider->list_dicts) (_provider, &n_dicts);
		for (size_t i = 0; i < n_dicts; i++)
		{
   			CHECK_EQUAL(1, (*_provider->dictionary_exists) (_provider, _dicts[i]));
        }
    }
}

