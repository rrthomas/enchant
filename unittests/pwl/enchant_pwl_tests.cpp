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
#define NOMINMAX //don't want windows to collide with std::min
#include <UnitTest++.h>
#include <enchant.h>
#include <enchant-provider.h>

#include "../EnchantDictionaryTestFixture.h"

struct EnchantPwl_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantPwl_TestFixture(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, languageTag)
    { }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryBeginsWithBOM
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryBeginsWithBOM_Successful)
{
	std::string Utf8Bom ("\xef\xbb\xbf");
	ExternalAddWordToDictionary(Utf8Bom + "cat");

	ReloadTestDictionary();

    CHECK( IsWordInDictionary("cat") );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryHasInvalidUtf8
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasInvalidUtf8Data_OnlyReadsToValidData)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<const std::string>::const_iterator bad = sWords.insert(sWords.begin()+2, "\xa5\xf1\x08"); //invalid utf8 data
  ExternalAddWordsToDictionary(sWords);

  ReloadTestDictionary();

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin(); itWord != bad; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*bad) );

  for(std::vector<const std::string>::const_iterator itWord = bad+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}
