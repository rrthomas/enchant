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

#include <UnitTest++/UnitTest++.h>
#include <enchant.h>
#include <vector>
#include <algorithm>

#include "EnchantDictionaryTestFixture.h"

static bool dictSuggestCalled;
std::string suggestWord;

static enum SuggestBehavior{
    returnNull,
    returnZero,
    returnFour,
    returnFourOneInvalidUtf8,
    returnFianceNfc
} suggestBehavior;

struct EnchantDictionarySuggestTestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionarySuggestTestFixtureBase(ConfigureHook userConfiguration, const std::string& languageTag="qaa"):
            EnchantDictionaryTestFixture(userConfiguration, languageTag)
    { 
        dictSuggestCalled = false;
        _suggestions = NULL;
        _pwl_suggestions = NULL;
        suggestWord = std::string();
        suggestBehavior = returnFour;
    }
    //Teardown
    ~EnchantDictionarySuggestTestFixtureBase()
    {
        FreeStringList(_suggestions);
        FreePwlStringList(_pwl_suggestions);
    }

    char** _suggestions;
    char** _pwl_suggestions;
};

static char **
MyMockDictionarySuggest (EnchantDict * dict, const char *const word, size_t len, size_t * out_n_suggs)
{
    dictSuggestCalled = true;
    suggestWord = std::string(word,len);
    *out_n_suggs = 0;
    char **sugg_arr = NULL;

    switch(suggestBehavior)
    {
        case returnNull:
            sugg_arr = NULL;
            break;
        case returnZero:
            sugg_arr = g_new0 (char *, *out_n_suggs + 1);
            break;
        case returnFianceNfc:
            *out_n_suggs = 1;
            sugg_arr = g_new0 (char *, *out_n_suggs + 1);
            sugg_arr[0] = g_strdup ("fianc\xc3\xa9");  // c3 a9 = utf8 for u00e9 = Latin small letter e with acute
            break;
        case returnFour:
            sugg_arr = MockDictionarySuggest(dict, word, len, out_n_suggs);
            break;
        case returnFourOneInvalidUtf8:
            sugg_arr = MockDictionarySuggest(dict, word, len, out_n_suggs);
            g_free(sugg_arr[0]);
            sugg_arr[0] = g_strdup ("\xa5\xf1\x08");
            break;
    }

    return sugg_arr;
}

static EnchantDict* MockProviderRequestSuggestMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = MyMockDictionarySuggest;
    return dict;
}

static void DictionarySuggest_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestSuggestMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}




struct EnchantDictionarySuggest_TestFixture_qaa : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggest_TestFixture_qaa():
            EnchantDictionarySuggestTestFixtureBase(DictionarySuggest_ProviderConfiguration, "qaa")
    { }
};

struct EnchantDictionarySuggest_TestFixture_qaaqaa : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggest_TestFixture_qaaqaa():
            EnchantDictionarySuggestTestFixtureBase(DictionarySuggest_ProviderConfiguration, "qaa,qaa")
    { }
};



static EnchantDict* MockProviderRequestNoSuggestMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = NULL;
    return dict;
}

static void DictionaryNoSuggest_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestNoSuggestMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionarySuggestNotImplemented_TestFixture_qaa : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggestNotImplemented_TestFixture_qaa():
            EnchantDictionarySuggestTestFixtureBase(DictionaryNoSuggest_ProviderConfiguration, "qaa")
    { }
};

struct EnchantDictionarySuggestNotImplemented_TestFixture_qaaqaa : EnchantDictionarySuggestTestFixtureBase
{
    //Setup
    EnchantDictionarySuggestNotImplemented_TestFixture_qaaqaa():
            EnchantDictionarySuggestTestFixtureBase(DictionaryNoSuggest_ProviderConfiguration, "qaa,qaa")
    { }
};

#define EnchantDictionarySuggest_TestFixture EnchantDictionarySuggest_TestFixture_qaa
#define EnchantDictionarySuggestNotImplemented_TestFixture EnchantDictionarySuggestNotImplemented_TestFixture_qaa
#include "suggest.i"

#undef EnchantDictionarySuggest_TestFixture
#define EnchantDictionarySuggest_TestFixture EnchantDictionarySuggest_TestFixture_qaaqaa
#undef EnchantDictionarySuggestNotImplemented_TestFixture
#define EnchantDictionarySuggestNotImplemented_TestFixture EnchantDictionarySuggestNotImplemented_TestFixture_qaaqaa
#include "suggest.i"
