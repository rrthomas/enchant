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

#ifndef EnchantDictionarySetError_TestFixture
#error EnchantDictionarySetError_TestFixture must be defined as the testfixture class to run these tests against
#endif

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantDictionarySetError_TestFixture, 
             SetErrorMessageOnDictionary)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_dict_set_error(_dict, expectedErrorMessage.c_str());

    CHECK_EQUAL(expectedErrorMessage, GetErrorMessage());
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantDictionarySetError_TestFixture, 
             SetErrorMessageOnDictionary_NullDictionary_NoErrorSet)
{
    enchant_dict_set_error(NULL, "Error message to display");

    CHECK_EQUAL(std::string(), GetErrorMessage());
}

TEST_FIXTURE(EnchantDictionarySetError_TestFixture, 
             SetErrorMessageOnDictionary_NullError_NoErrorSet)
{
    enchant_dict_set_error(_dict, NULL);

    CHECK_EQUAL(std::string(), GetErrorMessage());
}

TEST_FIXTURE(EnchantDictionarySetError_TestFixture, 
             SetErrorMessageOnDictionary_MessageCopied)
{
    std::string expectedErrorMessage("Error message to display");
    enchant_dict_set_error(_dict, expectedErrorMessage.c_str());

    expectedErrorMessage[0] = 'e';
    CHECK(expectedErrorMessage != GetErrorMessage());
}
