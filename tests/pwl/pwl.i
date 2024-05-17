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

#ifndef EnchantPwl_TestFixture
#error EnchantPwl_TestFixture must be defined as the testfixture class to run these tests against
#endif

#ifndef EnchantPwlWithDictSuggs_TestFixture
#error EnchantPwlWithDictSuggs_TestFixture must be defined as the testfixture class to run these tests against
#endif

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
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryBeginsWithBOM
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryBeginsWithBOM_Successful)
{
    const char* Utf8Bom = "\xef\xbb\xbf";

    FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "a");
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

    FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "a");
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

  FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "a");
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
