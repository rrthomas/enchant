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
             IsWordInDictionary_DictionaryHasInvalidUtf8Data_OnlyReadsValidLines)
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


////////////////////////////////////////////////////////////////////////////////////////////////
// Pwl Bugs
TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_DistanceUsesUnicodeChar)
{
	std::string puaWord("\xF4\x80\x80\x80ord"); // private use character 
	AddWordToDictionary(puaWord); //edit distance 1 using unichar; 4 using utf8

    std::vector<const std::string> suggestions = GetSuggestionsFromWord("word");

	CHECK( !suggestions.empty());

	if(!suggestions.empty()){
		CHECK_EQUAL(puaWord, suggestions[0]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Capitalization
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedAllCaps_OnlyAllCapsSuccessful)
{
  AddWordToDictionary("CIA");

  CHECK( IsWordInDictionary("CIA") );
  CHECK(!IsWordInDictionary("CIa") );
  CHECK(!IsWordInDictionary("Cia") );
  CHECK(!IsWordInDictionary("cia") );
  CHECK(!IsWordInDictionary("cIa") );

  CHECK( IsWordInSession("CIA") );
  CHECK(!IsWordInSession("CIa") );
  CHECK(!IsWordInSession("Cia") );
  CHECK(!IsWordInSession("cia") );
  CHECK(!IsWordInSession("cIa") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedTitle_lowerCaseAndMixedCaseNotSuccessful)
{
  AddWordToDictionary("Eric");

  CHECK( IsWordInDictionary("ERIC") );
  CHECK(!IsWordInDictionary("ERic") );
  CHECK( IsWordInDictionary("Eric") );
  CHECK(!IsWordInDictionary("eric") );
  CHECK(!IsWordInDictionary("eRic") );

  CHECK( IsWordInSession("ERIC") );
  CHECK(!IsWordInSession("ERic") );
  CHECK( IsWordInSession("Eric") );
  CHECK(!IsWordInSession("eric") );
  CHECK(!IsWordInSession("eRic") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_Addedlower_MixedCaseNotSuccessful)
{
  AddWordToDictionary("rice");

  CHECK( IsWordInDictionary("RICE") );
  CHECK(!IsWordInDictionary("RIce") );
  CHECK( IsWordInDictionary("Rice") );
  CHECK( IsWordInDictionary("rice") );
  CHECK(!IsWordInDictionary("rIce") );

  CHECK( IsWordInSession("RICE") );
  CHECK(!IsWordInSession("RIce") );
  CHECK( IsWordInSession("Rice") );
  CHECK( IsWordInSession("rice") );
  CHECK(!IsWordInSession("rIce") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedAllCapsOfTrueTitleCase_OnlyAllCapsSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedTrueTitleCase_lowerCaseAndMixedCaseNotSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedLowerOfTrueTitleCase_MixedCaseNotSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f3ie")); // u01f2 is Latin small letter dz

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}
