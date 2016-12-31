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
#include <glib.h>
#include <assert.h>

struct DictionarySuggest_TestFixture : Provider_TestFixture
{
    EnchantDict* _dict;
    std::vector<std::string> _addedWords;
    //Setup
    DictionarySuggest_TestFixture():_dict(NULL)
    { 
        _dict = GetFirstAvailableDictionary();
    }

    //Teardown
    ~DictionarySuggest_TestFixture()
    {
        ReleaseDictionary(_dict);
    }

    std::vector<std::string> GetSuggestionsFromWord(EnchantDict* dict, const std::string& word)
    {
        std::vector<std::string> result;
        if(dict && dict->suggest)
        {
            size_t cSuggestions;
            char** suggestions = (*dict->suggest)(dict, word.c_str(), word.size(), &cSuggestions);

            if(suggestions != NULL){
                result.insert(result.begin(), suggestions, suggestions+cSuggestions);
            }

            if(_provider->free_string_list){
                _provider->free_string_list(_provider, suggestions);
            }
        }
        
        return result;
    }

    std::vector<std::string> GetSuggestionsFromWord(const std::string& word)
    {
        return GetSuggestionsFromWord(_dict, word);
    }

    bool IsWordAllCaps(const std::string& word)
    {
	    const char* it, *itEnd;
	    bool hasCap = false;

            for(it = word.c_str(), itEnd = it+word.length();
            it < itEnd; it = g_utf8_next_char(it))
		    {
			    GUnicodeType type = g_unichar_type(g_utf8_get_char(it));
			    switch(type)
				    {
					    case G_UNICODE_UPPERCASE_LETTER:
						    hasCap = true;
						    break;
					    case G_UNICODE_TITLECASE_LETTER:
					    case G_UNICODE_LOWERCASE_LETTER:
						    return false;
				    }
		    }

	    return hasCap;
    }

    bool IsFirstLetterCapitalOrTitleCase(const std::string& word)
    {
	    gunichar ch;
	    GUnicodeType type;

            ch = g_utf8_get_char(word.c_str());
    	
	    type = g_unichar_type(ch);
	    if(type == G_UNICODE_UPPERCASE_LETTER || type == G_UNICODE_TITLECASE_LETTER)
		    return true;

	    return false;
    }

};

/////////////////////////////////////////////////////////////////////////////////////////////////
TEST_FIXTURE(DictionarySuggest_TestFixture, 
             Suggest_ReturnsSuggestions)
{
    if(_dict && _dict->suggest)
    {
      std::vector<std::string> suggestions = GetSuggestionsFromWord("fiance");
      CHECK(suggestions.size() != 0);
    }
}

TEST_FIXTURE(DictionarySuggest_TestFixture, 
             Suggest_HandlesNFC)
{
    EnchantDict* dict = GetDictionary("fr_FR");
    if(dict && dict->suggest)
    {
      std::vector<std::string> suggestions = GetSuggestionsFromWord(dict, Convert(L"fran\x00e7" L"ais")); //NFC latin small letter c with cedilla
      CHECK(suggestions.size() != 0);
    }
    ReleaseDictionary(dict);
}

TEST_FIXTURE(DictionarySuggest_TestFixture, 
             Suggest_HandlesNFD)
{
    EnchantDict* dict = GetDictionary("fr_FR");
    if(dict && dict->suggest)
    {
      std::vector<std::string> suggestions = GetSuggestionsFromWord(dict, Convert(L"franc\x0327" L"ais")); //NFD combining cedilla
      CHECK(suggestions.size() != 0);
    }
    ReleaseDictionary(dict);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Capitalization
TEST_FIXTURE(DictionarySuggest_TestFixture, 
             GetSuggestions_FromAllCaps_ResultsAllCaps)
{
    if(_dict && _dict->suggest)
    {
      std::vector<std::string> suggestions = GetSuggestionsFromWord("AAA");
      for(std::vector<std::string>::const_iterator i = suggestions.begin();
          i != suggestions.end();
          ++i)
      {
          CHECK(IsWordAllCaps(*i));
      }
    }
}

TEST_FIXTURE(DictionarySuggest_TestFixture, 
             GetSuggestions_FromTitle_ResultsTitleOrAllCaps)
{
    if(_dict && _dict->suggest)
    {
      std::vector<std::string> suggestions = GetSuggestionsFromWord("Aaa");
      for(std::vector<std::string>::const_iterator i = suggestions.begin();
          i != suggestions.end();
          ++i)
      {
          CHECK(IsFirstLetterCapitalOrTitleCase(*i));
      }
    }
}
