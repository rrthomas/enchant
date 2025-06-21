/* Copyright (c) 2007 Eric Scott Albright
 * Copyright (c) 2024 Reuben Thomas
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

#ifndef EnchantDictionaryCheck_TestFixture
#error EnchantDictionaryCheck_TestFixture must be defined as the testfixture class to run these tests against
#endif

#ifndef EnchantDictionaryCheckNotImplemented_TestFixture
#error EnchantDictionaryCheckNotImplemented_TestFixture must be defined as the testfixture class to run these tests against
#endif

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordExists_LenComputed_0)
{
    CHECK_EQUAL(0, enchant_dict_check(_dict, "hello", -1));
    CHECK(dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordExists_LenSpecified_0)
{
    CHECK_EQUAL(0, enchant_dict_check(_dict, "hellodisregard me", 5));
    CHECK(dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordDoesNotExist_LenComputed_1)
{
    CHECK_EQUAL(1, enchant_dict_check(_dict, "helo", -1));
    CHECK(dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordDoesNotExist_LenSpecified_1)
{
    CHECK_EQUAL(1, enchant_dict_check(_dict, "helodisregard me", 4));
    CHECK(dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordExistsInSession_0_DoesNotCallProvider)
{
    enchant_dict_add_to_session(_dict, "session", -1);
    CHECK_EQUAL(0, enchant_dict_check(_dict, "session", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordExistsInPersonal_0_DoesNotCallProvider)
{
    enchant_dict_add(_dict, "personal", -1);

    CHECK_EQUAL(0, enchant_dict_check(_dict, "personal", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordDoesExists_InBrokerPwlSession_0)
{
    enchant_dict_add_to_session(_pwl, "session", -1);
    CHECK_EQUAL(0, enchant_dict_check(_pwl, "session", -1));
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordDoesExists_InBrokerPwl_0)
{
    enchant_dict_add(_pwl, "personal", -1);
    CHECK_EQUAL(0, enchant_dict_check(_pwl, "personal", -1));
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordDoesNotExist_InBrokerPwl_1)
{
    CHECK_EQUAL(1, enchant_dict_check(_pwl, "session", -1));
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture, 
             EnchantDictionaryCheck_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_check(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_NullDictionary_Negative1)
{
    CHECK_EQUAL(-1, enchant_dict_check(NULL, "helo", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_NullWord_Negative1)
{
    CHECK_EQUAL(-1, enchant_dict_check(_dict, NULL, -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_EmptyWord_Negative1)
{
    CHECK_EQUAL(-1, enchant_dict_check(_dict, "", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordSize0_Negative1)
{
    CHECK_EQUAL(-1, enchant_dict_check(_dict, "helo", 0));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_InvalidUtf8Word_Negative1)
{
    CHECK_EQUAL(-1, enchant_dict_check(_dict, "\xa5\xf1\x08", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheckNotImplemented_TestFixture,
             EnchantDictionaryCheckNotImplemented_InBrokerPwl_1)
{
    CHECK_EQUAL(1, enchant_dict_check(_pwl, "hello", -1));
    CHECK(!dictCheckCalled);
}

TEST_FIXTURE(EnchantDictionaryCheck_TestFixture,
             EnchantDictionaryCheck_WordInDictionaryAndExclude_1)
{
    ExternalAddWordToExclude("hello");
    ExternalAddWordToDictionary("hello");

	ReloadTestDictionary();
	CHECK_EQUAL(1, enchant_dict_check(_dict, "hello", -1));
}
