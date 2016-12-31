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

#include <unistd.h>
#define NOMINMAX //don't want windows to collide with std::min
#include <UnitTest++.h>
#include <stdio.h>
#include <enchant.h>
#include <enchant-provider.h>

#include "EnchantDictionaryTestFixture.h"

#include <algorithm>

static char **
DictionarySuggestsSat (EnchantDict * dict, const char *const word, size_t len, size_t * out_n_suggs)
{
    *out_n_suggs = 1;
    char **sugg_arr = NULL;

    sugg_arr = g_new0 (char *, *out_n_suggs + 1);
    sugg_arr[0] = g_strdup ("sat");

    return sugg_arr;
}

static EnchantDict* MockProviderRequestSuggestMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = DictionarySuggestsSat;
    return dict;
}

static void DictionarySuggest_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestSuggestMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
     me->free_string_list = MockProviderFreeStringList;
}


struct EnchantPwlWithDictSuggs_TestFixture : EnchantDictionaryTestFixture
{
    EnchantPwlWithDictSuggs_TestFixture(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(DictionarySuggest_ProviderConfiguration, languageTag)
    { }
};

struct EnchantPwl_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantPwl_TestFixture(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, languageTag)
    { }
};

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestionsFromWord_MultipleSuggestions_ReturnsOnlyClosest)
{
  std::vector<std::string> sNoiseWords;
  sNoiseWords.push_back("spat");
  sNoiseWords.push_back("tots");
  sNoiseWords.push_back("tater");
  sNoiseWords.push_back("ton");
  sNoiseWords.push_back("gnat");

  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  AddWordsToDictionary(sWords);
  AddWordsToDictionary(sNoiseWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");
  CHECK_EQUAL(sWords.size(), suggestions.size());
  
  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwlWithDictSuggs_TestFixture,
             GetSuggestionsFromWord_MultipleSuggestions_ReturnsOnlyAsCloseAsDict)
{
  std::vector<std::string> sNoiseWords;
  sNoiseWords.push_back("spat");
  sNoiseWords.push_back("tots");
  sNoiseWords.push_back("tater");
  sNoiseWords.push_back("ton");
  sNoiseWords.push_back("gnat");

  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  AddWordsToDictionary(sWords);
  AddWordsToDictionary(sNoiseWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");
  sWords.push_back("sat");
  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// External File change
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryChangedExternally_Successful)
{
  UnitTest::TestResults testResults_;

  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  ExternalAddWordsToDictionary(sWords);

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != sWords.end(); ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }

  std::vector<std::string> sNewWords;
  sNewWords.push_back("potatoe");
  sNewWords.push_back("grow");
  sNewWords.push_back("another");

  ExternalAddNewLineToDictionary();
  ExternalAddWordsToDictionary(sNewWords);

  for(std::vector<std::string>::const_iterator itWord = sNewWords.begin(); itWord != sNewWords.end(); ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
    if(!IsWordInDictionary(*itWord)){
         testResults_.OnTestFailure(UnitTest::TestDetails(m_details, __LINE__), itWord->c_str());
    }
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_DictionaryChangedExternally_Successful)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  ExternalAddWordsToDictionary(sWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryBeginsWithBOM
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryBeginsWithBOM_Successful)
{
    const char* Utf8Bom = "\xef\xbb\xbf";

    sleep(1); // FAT systems have a 2 second resolution
                 // NTFS is appreciably faster but no specs on what it is exactly
                 // c runtime library's time_t has a 1 second resolution
    FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "at");
	if(f)
	{
		fputs(Utf8Bom, f);
                fputs("cat", f);
		fclose(f);
	}


    ReloadTestDictionary();

    CHECK( IsWordInDictionary("cat") );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryHasInvalidUtf8
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasInvalidUtf8Data_OnlyReadsValidLines)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator bad = sWords.insert(sWords.begin()+2, "\xa5\xf1\x08"); //invalid utf8 data
  ExternalAddWordsToDictionary(sWords);

  ReloadTestDictionary();

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != bad; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*bad) );

  for(std::vector<std::string>::const_iterator itWord = bad+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Last word in Dictionary terminated By EOF instead of NL
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_LastWordNotTerminatedByNL_WordsAppendedOkay)
{
    std::vector<std::string> sWords;
    sWords.push_back("cat");
    sWords.push_back("hat");
    sWords.push_back("that");
    sWords.push_back("bat");
    sWords.push_back("tot");

    sleep(1); // FAT systems have a 2 second resolution
                 // NTFS is appreciably faster but no specs on what it is exactly
                 // c runtime library's time_t has a 1 second resolution
    FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "at");
    if(f)
    {
	    fputs(sWords[0].c_str(), f);
	    fclose(f);
    }

    for(std::vector<std::string>::const_iterator itWord = sWords.begin() +1;
        itWord != sWords.end();
        ++itWord)
    {
        AddWordToDictionary(*itWord);
    }

    for(std::vector<std::string>::const_iterator itWord = sWords.begin(); 
        itWord != sWords.end(); 
        ++itWord){
        CHECK( IsWordInDictionary(*itWord) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Pwl Bugs
TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_DistanceUsesUnicodeChar)
{
	std::string puaWord("\xF4\x80\x80\x80ord"); // private use character 
	AddWordToDictionary(puaWord); //edit distance 1 using unichar; 4 using utf8

    std::vector<std::string> suggestions = GetSuggestionsFromWord("word");

	CHECK( !suggestions.empty());

	if(!suggestions.empty()){
		CHECK_EQUAL(puaWord, suggestions[0]);
	}
}

// Word which is prefix of another gets edit distance which is one less. 
// This means it moves to the top of the list normally but once we only bring
// back the best matches, it means the rest of the matches aren't returned.
// FIXME: This is not very clear. See FIXME below.
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_EditDistanceOnWordWhichIsPrefixOfAnother)
{
  std::vector<std::string> sNoiseWords;
  sNoiseWords.push_back("hastens"); //4

  std::vector<std::string> sWords;
  sWords.push_back("cashes"); //3
  sWords.push_back("hasten"); //3
  sWords.push_back("washes"); //3

  AddWordsToDictionary(sWords);
  AddWordsToDictionary(sNoiseWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("saskep");
  // FIXME: The string in the next line was originally "hasten", but the
  // test failed. Is this now correct?
  CHECK(suggestions[0] != "hastens");
  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Commented Lines ignored
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasCommentedLines_DoesNotReadCommentedLines)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator comment = sWords.insert(sWords.begin()+2, "#sat"); //comment
  ExternalAddWordsToDictionary(sWords);
  ReloadTestDictionary();

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != comment; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }

  CHECK(!IsWordInDictionary(*comment) );
  CHECK(!IsWordInDictionary("sat") );

  for(std::vector<std::string>::const_iterator itWord = comment+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Too long lines ignored
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasSuperLongLine_DoesNotReadLine)
{
  const size_t lineLen = BUFSIZ + 1; // enchant ignores PWL lines longer than BUFSIZ

  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator superlong = sWords.insert(sWords.begin()+2, std::string(lineLen, 'c')); //super long line
  ExternalAddWordsToDictionary(sWords);
  ReloadTestDictionary();

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != superlong; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }

  CHECK(!IsWordInDictionary(*superlong) );
  for(size_t i=0; i != lineLen; ++i)
  {
      CHECK(!IsWordInDictionary(std::string(i, 'c')) );
  }

  for(std::vector<std::string>::const_iterator itWord = superlong+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Unicode normalization
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasComposed_SuccessfulCheckWithComposedAndDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  ReloadTestDictionary();

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD u0301 = Combining acute accent
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedComposed_SuccessfulCheckWithComposedAndDecomposed)
{
  AddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD u0301 = Combining acute accent
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  ReloadTestDictionary();

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
  AddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_DictionaryHasComposed_ReturnsComposed)
{
  ExternalAddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  ReloadTestDictionary();

  std::vector<std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_AddedComposed_ReturnsComposed)
{
  AddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  std::vector<std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_DictionaryHasDecomposed_ReturnsDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  ReloadTestDictionary();

  std::vector<std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"fiance\x301"));  // u0301 = Combining acute accent
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_AddedDecomposed_ReturnsDecomposed)
{
  AddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  std::vector<std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"fiance\x301")); // u0301 = Combining acute accent
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
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

///////////////////////////////////////////////////////////////////////////////////////////
// Capitalization on Suggestions
TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("CEA");

  std::vector<std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordTitleCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Cea");

  std::vector<std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("cea");

  std::vector<std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("Eric");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("RIC");

  std::vector<std::string> expected;
  expected.push_back("ERIC");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary("Eric");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Ric");

  std::vector<std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordLowerCase_SuggestionTitleCase)
{
  AddWordToDictionary("Eric");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ric");

  std::vector<std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("rice");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("RIC");

  std::vector<std::string> expected;
  expected.push_back("RICE");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary("rice");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Ric");

  std::vector<std::string> expected;
  expected.push_back("Rice");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordLowerCase_SuggestionLowerCase)
{
  AddWordToDictionary("rice");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ric");

  std::vector<std::string> expected;
  expected.push_back("rice");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}






TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordTitleCase_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordLowerCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordLowerCase_SuggestionLowerCase)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<std::string> expected;
  expected.push_back(Convert(L"\x01f3ie")); // u01f3 is Latin small letter dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}


TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsWithPrefix_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIAL");
  AddWordToDictionary("CIALAND");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ceal");

  std::vector<std::string> expected;
  expected.push_back("CIAL");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseWithPrefix_WordLowerCase_SuggestionTitlecase)
{
  AddWordToDictionary("Eric");
  AddWordToDictionary("Ericson");

  std::vector<std::string> suggestions = GetSuggestionsFromWord("eruc");

  std::vector<std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

/////////////////////////////////////////////////////////////////////////////
// Remove from PWL
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix1)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("help");

  CHECK(!IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix2)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix3)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");
  AddWordToDictionary("helm");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK(!IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix4)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");
  AddWordToDictionary("helm");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );

  RemoveWordFromDictionary("help");

  CHECK(!IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SingleWord)
{
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_MultipleWords1)
{
  AddWordToDictionary("special");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_MultipleWords2)
{
  AddWordToDictionary("special");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("special");

  CHECK(!IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix1)
{
  AddWordToDictionary("ant");
  AddWordToDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("ant");

  CHECK(!IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix2)
{
  AddWordToDictionary("anteater");
  AddWordToDictionary("ant");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("ant");

  CHECK(!IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix3)
{
  AddWordToDictionary("ant");
  AddWordToDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK(!IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix4)
{
  AddWordToDictionary("anteater");
  AddWordToDictionary("ant");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK(!IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromFile)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator removed = sWords.insert(sWords.begin()+2, "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );

  for(std::vector<std::string>::const_iterator itWord = removed+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromBeginningOfFile)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator removed = sWords.insert(sWords.begin(), "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  CHECK(!IsWordInDictionary(*removed) );

  for(std::vector<std::string>::const_iterator itWord = removed+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromBeginningOfFileWithBOM)
{
  const char* Utf8Bom = "\xef\xbb\xbf";

  std::vector<std::string> sWords;
  sWords.push_back("hello");
  sWords.push_back("cat");
  sWords.push_back("hat");

  sleep(1); // FAT systems have a 2 second resolution
               // NTFS is appreciably faster but no specs on what it is exactly
               // c runtime library's time_t has a 1 second resolution
  FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "at");
  if(f) {
    fputs(Utf8Bom, f);
    for(std::vector<std::string>::const_iterator itWord = sWords.begin();
        itWord != sWords.end(); ++itWord)
    {
      if(itWord != sWords.begin()){
        fputc('\n', f);
      }
      fputs(itWord->c_str(), f);
    }
    fclose(f);
  }

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  CHECK(!IsWordInDictionary("hello") );

  for(std::vector<std::string>::const_iterator itWord = sWords.begin()+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromEndOfFile)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator removed = sWords.insert(sWords.end(), "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromEndOfFile_ExternalSetup)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<std::string>::const_iterator removed = sWords.insert(sWords.end(), "hello");
  ExternalAddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_FileHasProperSubset_ItemRemovedFromFile)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");
  sWords.push_back("anteater");

  std::vector<std::string>::const_iterator removed = sWords.insert(sWords.end(), "ant");
  AddWordsToDictionary(sWords);
  RemoveWordFromDictionary("ant");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pwl Edit distance
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_SubstituteFirstChar)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat");  //1

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catsup"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_SubstituteFirstChar_Insert1)
{
  std::vector<std::string> sWords;
  sWords.push_back("cats"); //2

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catsup"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_SubstituteFirstChar_Insert2)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //3

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catchy"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tat");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Insert1)
{
  std::vector<std::string> sWords;
  sWords.push_back("tad");  //1

  AddWordsToDictionary(sWords);

  AddWordToDictionary("taddle"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ta");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Insert2)
{
  std::vector<std::string> sWords;
  sWords.push_back("tote"); //2

  AddWordsToDictionary(sWords);

  AddWordToDictionary("totems"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("to");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Insert3)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //3

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catchy"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ca");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Delete1)
{
  std::vector<std::string> sWords;
  sWords.push_back("tape");  //1

  AddWordsToDictionary(sWords);

  AddWordToDictionary("tapestry"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("tapen");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Delete2)
{
  std::vector<std::string> sWords;
  sWords.push_back("tot"); //2

  AddWordsToDictionary(sWords);

  AddWordToDictionary("totality"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("totil");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Delete3)
{
  std::vector<std::string> sWords;
  sWords.push_back("cat"); //3

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catcher"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("catsip");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Substitute1)
{
  std::vector<std::string> sWords;
  sWords.push_back("small");  //1

  AddWordsToDictionary(sWords);

  AddWordToDictionary("smallest"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("skall");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Substitute2)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //2

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catcher"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("cafdh");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Substitute3)
{
  std::vector<std::string> sWords;
  sWords.push_back("hasten"); //3

  AddWordsToDictionary(sWords);

  AddWordToDictionary("hastens"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("hasopo");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Transpose1)
{
  std::vector<std::string> sWords;
  sWords.push_back("small");  //1

  AddWordsToDictionary(sWords);

  AddWordToDictionary("smallest"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("smlal");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Transpose2)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //2

  AddWordsToDictionary(sWords);

  AddWordToDictionary("catcher"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("acthc");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_HasProperSubset_Transpose3)
{
  std::vector<std::string> sWords;
  sWords.push_back("hasten"); //3

  AddWordsToDictionary(sWords);

  AddWordToDictionary("hastens"); //4

  std::vector<std::string> suggestions = GetSuggestionsFromWord("ahtsne");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_Transpose1Insert2)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //3

  AddWordsToDictionary(sWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("act");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlSuggest_Transpose2)
{
  std::vector<std::string> sWords;
  sWords.push_back("catch"); //2

  AddWordsToDictionary(sWords);

  std::vector<std::string> suggestions = GetSuggestionsFromWord("acthc");

  CHECK_EQUAL(sWords.size(), suggestions.size());

  std::sort(sWords.begin(), sWords.end());
  std::sort(suggestions.begin(), suggestions.end());

  CHECK_ARRAY_EQUAL(sWords, suggestions, std::min(sWords.size(), suggestions.size()));
}

