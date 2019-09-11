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
#include "EnchantDictionaryTestFixture.h"

struct EnchantDictionaryIsAdded_TestFixture : EnchantDictionaryTestFixture
{};

/**
 * enchant_dict_is_added
 * @dict: A non-null #EnchantDict
 * @word: The word you wish to see if it's in your session
 * @len: the byte length of @word, or -1 for strlen (@word)
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_AddedToSession_1)
{
    enchant_dict_add_to_session(_dict, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_added(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_Added_1)
{
    enchant_dict_add(_dict, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_added(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_NotAdded_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_OnBrokerPwl_AddedToSession_1)
{
    enchant_dict_add_to_session(_pwl, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_added(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_OnBrokerPwl_Added_1)
{
    enchant_dict_add(_pwl, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_added(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_OnBrokerPwl_NotAdded_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_is_added(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture,
             EnchantDictionaryIsAdded_NullDictionary_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(NULL, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture,
             EnchantDictionaryIsAdded_NullWord_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_dict, NULL, -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture,
             EnchantDictionaryIsAdded_EmptyWord_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_dict, "", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture,
             EnchantDictionaryIsAdded_WordSize0_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_dict, "hello", 0));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture,
             EnchantDictionaryIsAdded_InvalidUtf8Word_0)
{
    CHECK_EQUAL(0, enchant_dict_is_added(_dict, "\xa5\xf1\x08", -1));
}

TEST_FIXTURE(EnchantDictionaryIsAdded_TestFixture, 
             EnchantDictionaryIsAdded_WordExistsInPwlAndExclude_0)
{
    ExternalAddWordToDictionary("hello");
    ExternalAddWordToExclude("hello");

	ReloadTestDictionary();
	CHECK_EQUAL(0, enchant_dict_is_added(_dict, "hello", -1));
}

