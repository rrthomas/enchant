/* Copyright (c) 2008 Eric Scott Albright
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

#include "../unittest_enchant_providers.h"
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>

struct DictionaryCheck_TestFixture : Provider_TestFixture
{
    typedef std::multimap<EnchantProviderDict*, std::string> AddedWordsByDict;
    EnchantProviderDict* _dict;
    const char *_provider_name;
    AddedWordsByDict _addedWordsByDict;

    //Setup
    DictionaryCheck_TestFixture():_dict(NULL)
    { 
        _dict = GetDefaultDictionary();
        /* FIXME: hspell does not consider non-Hebrew letters to be valid letters */
        if (_dict) {
          _provider_name = _provider->identify(_provider);
          if (strcmp(_provider_name, "hspell") == 0) {
            ReleaseDictionary(_dict);
            _dict = NULL;
          }
        }
    }

    //Teardown
    ~DictionaryCheck_TestFixture()
    {
        ReleaseDictionary(_dict);
    }

    virtual void ReleaseDictionary(EnchantProviderDict* dict){
        std::pair<AddedWordsByDict::const_iterator,
                  AddedWordsByDict::const_iterator>
                  addedWords = _addedWordsByDict.equal_range(dict);

        AddedWordsByDict::const_iterator it;
        _addedWordsByDict.erase(dict);
        Provider_TestFixture::ReleaseDictionary(dict);
    }

    bool IsWordInDictionary(const std::string& word)
    {
        return IsWordInDictionary(_dict, word);
    }

    bool IsWordInDictionary(const std::u8string& word)
    {
        return IsWordInDictionary(_dict, word);
    }

    static bool IsWordInDictionary(EnchantProviderDict* dict, const std::string& word)
    {
        assert(dict && dict->check); // tests must check this before calling

        return (*dict->check)(dict, word.c_str(), word.length()) == 0; //check returns 0 when successful and 1 when not successful
    }

    static bool IsWordInDictionary(EnchantProviderDict* dict, const std::u8string& word)
    {
        assert(dict && dict->check); // tests must check this before calling

        return (*dict->check)(dict, reinterpret_cast<const char *>(word.c_str()), word.length()) == 0; //check returns 0 when successful and 1 when not successful
    }

    bool AddWordToDictionary(const std::string& word)
    {
        return AddWordToDictionary(_dict, word);
    }

    bool AddWordToDictionary(const std::u8string& word)
    {
        return AddWordToDictionary(_dict, word);
    }

    bool AddWordToDictionary(EnchantProviderDict* dict, const char *word, size_t len) {
        if(dict == NULL)
            return false;

        if(dict->check(dict, word, len))
            return true;

        // prefer adding it to the session so it will get automatically removed
        if(dict->add_to_session)
        {
            (*dict->add_to_session) (dict, word, len);
            if(dict->check(dict, word, len))
                return true;
        }

        return false;
    }

    bool AddWordToDictionary(EnchantProviderDict* dict, const std::string& word)
    {
        return AddWordToDictionary(dict, word.c_str(), word.length());
    }

    bool AddWordToDictionary(EnchantProviderDict* dict, const std::u8string& word)
    {
        return AddWordToDictionary(dict, reinterpret_cast<const char *>(word.c_str()), word.length());
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// Unicode normalization
TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedComposed_SuccessfulCheckWithComposedAndDecomposed)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary(u8"fiancé" u8"deleteme"))
      {
          CHECK( IsWordInDictionary(u8"fianc\u00e9" u8"deleteme") ); //NFC
          CHECK( IsWordInDictionary(u8"fiance\u0301" u8"deleteme") ); //NFD u0301 = Combining acute accent
      }
    }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary(u8"fiance\u0301" u8"deletethis")) // u0301 = Combining acute accent
      {
          CHECK( IsWordInDictionary(u8"fianc\u00e9" u8"deletethis") ); //NFC
          CHECK( IsWordInDictionary(u8"fiance\u0301" u8"deletethis") ); //NFD
      }
    }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_SuccessfulCheckWithComposedAndDecomposed)
{
    EnchantProviderDict* dict = GetDictionary("fr_FR");
    if(dict && dict->check)
    {
        CHECK( IsWordInDictionary(dict, u8"Fran\u00e7ais") ); //NFC latin small letter c with cedilla
        CHECK( IsWordInDictionary(dict, u8"Franc\u0327ais") ); //NFD combining cedilla
    }
    ReleaseDictionary(dict);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Capitalization
TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedAllCaps_OnlyAllCapsSuccessful)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary("ZYX"))
      {
          CHECK( IsWordInDictionary("ZYX") );
          CHECK(!IsWordInDictionary("ZYx") );
          CHECK(!IsWordInDictionary("Zyx") );
          CHECK(!IsWordInDictionary("zyx") );
	  CHECK(!IsWordInDictionary("zYx") );
      }
    }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedTitle_lowerCaseAndMixedCaseNotSuccessful)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary("Zxyz"))
      {
          CHECK( IsWordInDictionary("ZXYZ") );
          CHECK(!IsWordInDictionary("ZXyz") );
          CHECK( IsWordInDictionary("Zxyz") );
          CHECK(!IsWordInDictionary("zxyz") );
          CHECK(!IsWordInDictionary("zXyz") );
      }
   }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_Addedlower_MixedCaseNotSuccessful)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary("zyxz"))
      {
          CHECK( IsWordInDictionary("ZYXZ") );
          CHECK(!IsWordInDictionary("ZYxz") );
          CHECK( IsWordInDictionary("Zyxz") );
          CHECK( IsWordInDictionary("zyxz") );
          CHECK(!IsWordInDictionary("zYxz") );
      }
    }
}
