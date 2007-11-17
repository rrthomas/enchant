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
#include "../EnchantDictionaryTestFixture.h"

struct EnchantDictionaryIsInSession_TestFixture : EnchantDictionaryTestFixture
{};

/**
 * enchant_dict_is_in_session
 * @dict: A non-null #EnchantDict
 * @word: The word you wish to see if it's in your session
 * @len: the byte length of @word, or -1 for strlen (@word)
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_InSession_1)
{
    enchant_dict_add_to_session(_dict, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_in_session(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_InPersonal_1)
{
    enchant_dict_add_to_pwl(_dict, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_in_session(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_NotInSession_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_dict, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_OnBrokerPwl_InSession_1)
{
    enchant_dict_add_to_session(_pwl, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_in_session(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_OnBrokerPwl_InPersonal_1)
{
    enchant_dict_add_to_pwl(_pwl, "hello", -1);

    CHECK_EQUAL(1, enchant_dict_is_in_session(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_OnBrokerPwl_NotInSession_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_pwl, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture, 
             EnchantDictionaryIsInSession_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_is_in_session(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture,
             EnchantDictionaryIsInSession_NullDictionary_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(NULL, "hello", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture,
             EnchantDictionaryIsInSession_NullWord_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_dict, NULL, -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture,
             EnchantDictionaryIsInSession_EmptyWord_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_dict, "", -1));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture,
             EnchantDictionaryIsInSession_WordSize0_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_dict, "hello", 0));
}

TEST_FIXTURE(EnchantDictionaryIsInSession_TestFixture,
             EnchantDictionaryIsInSession_InvalidUtf8Word_0)
{
    CHECK_EQUAL(0, enchant_dict_is_in_session(_dict, "\xa5\xf1\x08", -1));
}
