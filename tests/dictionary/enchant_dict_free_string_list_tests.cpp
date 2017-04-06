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

struct EnchantDictFreeStringList_TestFixture : EnchantDictionaryTestFixture
{
    //setup
    EnchantDictFreeStringList_TestFixture()
    {
       _string_list = enchant_dict_suggest (_dict, "foo", -1, NULL);
       _pwl_string_list = enchant_dict_suggest (_pwl, "foo", -1, NULL);
    }
    //Teardown
    ~EnchantDictFreeStringList_TestFixture()
    {
        FreeStringList(_string_list);
        FreeStringList(_pwl_string_list);
    }
    char ** _string_list;
    char ** _pwl_string_list;
};

/**
 * enchant_dict_free_string_list
 * @dict: A non-null #EnchantDict
 * @string_list: A non-null string list returned from enchant_dict_suggest
 *
 * Releases the string list
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictFreeStringList_TestFixture,
             EnchantDictFreeStringList)
{
    enchant_dict_free_string_list(_dict, _string_list);
    _string_list = NULL;
}

TEST_FIXTURE(EnchantDictFreeStringList_TestFixture,
             EnchantDictFreeStringListOnPwl)
{
    enchant_dict_free_string_list(_pwl, _string_list);
    _string_list = NULL;
}

TEST_FIXTURE(EnchantDictFreeStringList_TestFixture, 
             EnchantDictFreeStringList_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockDictionary("something bad happened");

  enchant_dict_free_string_list(_dict, _string_list);
  _string_list = NULL;

  CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictFreeStringList_TestFixture,
             EnchantDictFreeStringList_NullDict_DoNothing)
{
    enchant_dict_free_string_list(NULL, _string_list);
}

TEST_FIXTURE(EnchantDictFreeStringList_TestFixture,
             EnchantDictFreeStringList_NullStringList_DoNothing)
{
    enchant_dict_free_string_list(_dict, NULL);
}
