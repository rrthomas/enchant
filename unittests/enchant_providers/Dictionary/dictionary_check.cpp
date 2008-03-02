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
#include <UnitTest++.h>

#include "../unittest_enchant_providers.h"
#include <vector>
#include <map>
#include <glib.h>
#include <assert.h>

struct DictionaryCheck_TestFixture : Provider_TestFixture
{
    typedef std::multimap<EnchantDict*, std::string> AddedWordsByDict;
    EnchantDict* _dict;
    AddedWordsByDict _addedWordsByDict;
    //Setup
    DictionaryCheck_TestFixture():_dict(NULL)
    { 
        _dict = GetFirstAvailableDictionary();
    }

    //Teardown
    ~DictionaryCheck_TestFixture()
    {
        ReleaseDictionary(_dict);
    }

    virtual void ReleaseDictionary(EnchantDict* dict){
        std::pair<AddedWordsByDict::const_iterator,
                  AddedWordsByDict::const_iterator>
                  addedWords = _addedWordsByDict.equal_range(dict);

        AddedWordsByDict::const_iterator it;
        for(it = addedWords.first; it != addedWords.second; ++it)
        {
            if(dict->add_to_exclude)
            {
                (*dict->add_to_exclude)(dict, it->second.c_str(), it->second.length());
            }
        }
        _addedWordsByDict.erase(dict);
        Provider_TestFixture::ReleaseDictionary(dict);
    }

    bool IsWordInDictionary(const std::string& word)
    {
        return IsWordInDictionary(_dict, word);
    }

    static bool IsWordInDictionary(EnchantDict* dict, const std::string& word)
    {
        assert(dict && dict->check); // tests must check this before calling

        return (*dict->check)(dict, word.c_str(), word.length()) == 0; //check returns 0 when successful and 1 when not successful
    }

    bool AddWordToDictionary(const std::string& word)
    {
        return AddWordToDictionary(_dict, word);
    }

    bool AddWordToDictionary(EnchantDict* dict, const std::string& word)
    {
        if(dict == NULL)
        {
            return false;
        }

        if(IsWordInDictionary(dict, word))
        {
            return true;
        }

        // prefer adding it to the session so it will get automatically removed
        if(dict->add_to_session)
        {
            (*dict->add_to_session) (dict, word.c_str(), word.length());
            if(IsWordInDictionary(word))
            {
                return true;
            }
        }

        if(dict->add_to_personal)
        {
            _addedWordsByDict.insert(std::pair<EnchantDict*, std::string>(dict,word));
            (*dict->add_to_personal) (dict, word.c_str(), word.length());
            if(IsWordInDictionary(word))
            {
                return true;
            }
        }

        return false;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// Unicode normalization
TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedComposed_SuccessfulCheckWithComposedAndDecomposed)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary(Convert(L"fianc\x00e9" L"deleteme"))) // u00e9 = Latin small letter e with acute
      {
          CHECK( IsWordInDictionary(Convert(L"fianc\x00e9" L"deleteme")) ); //NFC
          CHECK( IsWordInDictionary(Convert(L"fiance\x0301" L"deleteme")) ); //NFD u0301 = Combining acute accent
      }
    }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_AddedDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
    if(_dict && _dict->check)
    {
      if(AddWordToDictionary(Convert(L"fiance\x0301" L"deletethis"))) // u0301 = Combining acute accent
      {
          CHECK( IsWordInDictionary(Convert(L"fianc\x00e9" L"deletethis")) ); //NFC
          CHECK( IsWordInDictionary(Convert(L"fiance\x0301" L"deletethis")) ); //NFD
      }
    }
}

TEST_FIXTURE(DictionaryCheck_TestFixture, 
             IsWordInDictionary_SuccessfulCheckWithComposedAndDecomposed)
{
    EnchantDict* dict = GetDictionary("fr_FR");
    if(dict && dict->check)
    {
        CHECK( IsWordInDictionary(dict, Convert(L"Fran\x00e7" L"ais")) ); //NFC latin small letter c with cedilla
        CHECK( IsWordInDictionary(dict, Convert(L"Franc\x0327" L"ais")) ); //NFD combining cedilla
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
