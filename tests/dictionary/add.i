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

#ifndef EnchantDictionaryAdd_TestFixture
#error EnchantDictionaryAdd_TestFixture must be defined as the testfixture class to run these tests against
#endif

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInDictionary)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInDictionary("hello"));
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInSession)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInSession("hello"));
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordAddedToEnchantPwlFile)
{
    CHECK(!PersonalWordListFileHasContents());
    enchant_dict_add(_dict, "hello", -1);
    CHECK(PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_WordExistsInExclude_RemovedFromExcludeAddedToPersonal)
{
    enchant_dict_remove(_dict, "personal", -1);
    CHECK(ExcludeFileHasContents());
    enchant_dict_add(_dict, "personal", -1);
    CHECK(!ExcludeFileHasContents());
    CHECK(PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_InBrokerPwl)
{
    enchant_dict_add(_pwl, "personal", -1);
    CHECK(!PersonalWordListFileHasContents());
}

TEST_FIXTURE(EnchantDictionaryAdd_TestFixture,
             EnchantDictionaryAdd_IsPermanent)
{
    enchant_dict_add(_dict, "hello", -1);
    CHECK(IsWordInDictionary("hello"));

    ReloadTestDictionary();

    CHECK(IsWordInDictionary("hello"));
}


TEST_FIXTURE(EnchantDictionaryAdd_TestFixture, 
             EnchantDictionaryAdd_HasPreviousError_ErrorCleared)
{
    SetErrorOnMockDictionary("something bad happened");

    enchant_dict_add(_dict, "hello", -1);
    CHECK_EQUAL((void*)NULL, (void*)enchant_dict_get_error(_dict));
}
