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
#include <UnitTest++/UnitTest++.h>
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
}


struct EnchantPwlWithDictSuggs_TestFixture_qaa : EnchantDictionaryTestFixture
{
    EnchantPwlWithDictSuggs_TestFixture_qaa(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(DictionarySuggest_ProviderConfiguration, languageTag)
    { }
};

struct EnchantPwl_TestFixture_qaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantPwl_TestFixture_qaa(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, languageTag)
    { }
};

struct EnchantPwlWithDictSuggs_TestFixture_qaaqaa : EnchantDictionaryTestFixture
{
    EnchantPwlWithDictSuggs_TestFixture_qaaqaa(const std::string& languageTag="qaa,qaa"):
        EnchantDictionaryTestFixture(DictionarySuggest_ProviderConfiguration, languageTag)
    { }
};

struct EnchantPwl_TestFixture_qaaqaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantPwl_TestFixture_qaaqaa(const std::string& languageTag="qaa,qaa"):
        EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, languageTag)
    { }
};

#define EnchantPwl_TestFixture EnchantPwl_TestFixture_qaa
#define EnchantPwlWithDictSuggs_TestFixture EnchantPwlWithDictSuggs_TestFixture_qaa
#include "enchant_pwl_tests.i"

#undef EnchantPwl_TestFixture
#define EnchantPwl_TestFixture EnchantPwl_TestFixture_qaaqaa
#undef EnchantPwlWithDictSuggs_TestFixture
#define EnchantPwlWithDictSuggs_TestFixture EnchantPwlWithDictSuggs_TestFixture_qaaqaa
#include "enchant_pwl_tests.i"
