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

#ifndef __ENCHANTDICTIONARYTESTFIXTURE
#define __ENCHANTDICTIONARYTESTFIXTURE

#include "EnchantBrokerTestFixture.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>

static EnchantDict*
MockProviderRequestEmptyMockDictionary(EnchantProvider *me, const char *)
{
    EnchantDict* dict = enchant_broker_new_dict(me->owner);
    dict->user_data = NULL;
    dict->check = NULL;
    dict->suggest = NULL;
    dict->add_to_session = NULL;
	
    return dict;
}

static void EmptyDictionary_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestEmptyMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}


static char**
MockDictionarySuggest (EnchantDict * , 
                       const char *const word,
		               size_t len, 
                       size_t * out_n_suggs)
{
    *out_n_suggs = 4;

    char **sugg_arr = g_new0 (char *, *out_n_suggs + 1);
    for(size_t i=0; i<*out_n_suggs;++i){
        if(len == -1) {
            sugg_arr[i] = g_strdup (word);
        }
        else {
            sugg_arr[i] = g_strndup (word, (gsize)len);
        }
        sugg_arr[i][0] = 'a' + (char)i;
    }
    return sugg_arr;
}

static EnchantDict*
MockProviderRequestBasicMockDictionary(EnchantProvider *me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestEmptyMockDictionary(me, tag);
    dict->suggest = MockDictionarySuggest;
    return dict;
}


static void BasicDictionary_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestBasicMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryTestFixture : EnchantBrokerTestFixture
{
    EnchantDict* _dict;
    EnchantDict* _pwl;
    std::string _pwlFileName;
    std::string origLangEnv;
    bool hasLangEnv;
    std::string languageTag;

    //Setup
    EnchantDictionaryTestFixture(ConfigureHook userConfiguration=BasicDictionary_ProviderConfiguration, const std::string& languageTag="qaa"):
        EnchantBrokerTestFixture(userConfiguration),
        languageTag(languageTag)
    {
        InitializeTestDictionary();
        _pwl = RequestPersonalDictionary();
        _pwlFileName = GetLastPersonalDictionaryFileName();

        hasLangEnv = (g_getenv("LANG") != NULL);
        if(hasLangEnv)
        {
            origLangEnv = std::string(g_getenv("LANG"));
        }
    }

    //Teardown
    ~EnchantDictionaryTestFixture(){
        FreeTestDictionary();
        FreeDictionary(_pwl);
        ResetLocale();
    }

    void SetLocale(const std::string& locale)    
    {
        g_setenv("LANG", locale.c_str(), TRUE);
    }

    void ResetLocale()
    {
        if(hasLangEnv)
        {
            g_setenv("LANG", origLangEnv.c_str(), TRUE);
        }
        else{
            g_unsetenv("LANG");
        }
    }

    void ReloadTestDictionary()
    {
        FreeTestDictionary();
        InitializeTestDictionary();
    }

    void InitializeTestDictionary()
    {
        _dict = enchant_broker_request_dict(_broker, languageTag.c_str());
    }

    void FreeTestDictionary()
    {
        FreeDictionary(_dict);
    }

    bool PersonalWordListFileHasContents()
    {
        return FileHasContents(GetPersonalDictFileName());
    }

    bool ExcludeFileHasContents()
    {
        return FileHasContents(GetExcludeDictFileName());
    }

    bool BrokerPWLFileHasContents()
    {
        return FileHasContents(_pwlFileName);
    }

    bool FileHasContents(const std::string & filename)
    {
        bool hasContents = false;
        int fd = g_open(filename.c_str(), O_RDONLY, S_IREAD );
        if(fd == -1){
            return false;
        }

        char c;
        while(read(fd, &c, 1) == 1 && !hasContents){
            switch(c)
            {
            case '\n':
            case '\r':
            case ' ':
                break;
            default:
                hasContents = true;
            }
        }
        close(fd);

        return hasContents;
    }

    std::string GetPersonalDictFileName(){
        return AddToPath(GetTempUserEnchantDir(), "qaa.dic");
    }

    std::string GetExcludeDictFileName(){
        return AddToPath(GetTempUserEnchantDir(), "qaa.exc");
    }

    void SetErrorOnMockDictionary(const std::string& error)
    {
        enchant_dict_set_error(_dict, error.c_str());
    }

    void FreeStringList(char** list)
    {
        if(list)
        {
            enchant_dict_free_string_list(_dict, list);
        }
    }

    void FreePwlStringList(char** list)
    {
        if(list)
        {
            enchant_dict_free_string_list(_pwl, list);
        }
    }

    bool IsWordInSession(const std::string& word){
        return enchant_dict_is_added(_dict, word.c_str(), word.size())!=0;
    }

    bool IsWordInDictionary(const std::string& word){
        return enchant_dict_check(_dict, word.c_str(), word.size())==0;
    }

    void RemoveWordFromDictionary(const std::string& word)
    {
        enchant_dict_remove(_dict, word.c_str(), word.size());
    }

    void AddWordToDictionary(const std::string& word)
    {
	enchant_dict_add(_dict, word.c_str(), word.size());
    }

    void AddWordsToDictionary(const std::vector<std::string>& sWords)
    {
        for(std::vector<std::string>::const_iterator itWord = sWords.begin();
                                                              itWord != sWords.end();
                                                              ++itWord){
            AddWordToDictionary(*itWord);
        }
    }

    void ExternalAddWordToDictionary(const std::string& word)
    {
        ExternalAddWordToFile(word, GetPersonalDictFileName());
    }

    void ExternalAddWordToExclude(const std::string& word)
    {
        ExternalAddWordToFile(word, GetExcludeDictFileName());
    }

    static void ExternalAddWordToFile(const std::string& word, const std::string& filename)
    {
        FILE * f = g_fopen(filename.c_str(), "a");
	if(f)
	{
            fputc('\n', f);
            fputs(word.c_str(), f);
            fclose(f);
	}
    }

    void ExternalAddNewLineToDictionary()
    {
        FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "a");
        if(f)
        {
            fputc('\n', f);
            fclose(f);
        }
    }

    void ExternalAddWordsToDictionary(const std::vector<std::string>& sWords)
    {
        FILE * f = g_fopen(GetPersonalDictFileName().c_str(), "a");
        if(f)
        {
            for (std::vector<std::string>::const_iterator itWord = sWords.begin();
                itWord != sWords.end(); ++itWord)
            {
                if (itWord != sWords.begin()) {
                    fputc('\n', f);
                }
                fputs(itWord->c_str(), f);
            }
            fclose(f);
        }
    }

    std::vector<std::string> GetExpectedSuggestions(const std::string& s, size_t begin = 0)
    {
        size_t cSuggestions;
        char** expectedSuggestions = MockDictionarySuggest (_dict, 
                                                            s.c_str(),
		                                                    s.size(), 
                                                            &cSuggestions);

        std::vector<std::string> result;
        if(expectedSuggestions != NULL && begin < cSuggestions){
            result.insert(result.begin(), expectedSuggestions+begin, expectedSuggestions+cSuggestions);
            g_strfreev(expectedSuggestions);
        }

        return result;
    }


    std::vector<std::string> GetSuggestionsFromWord(const std::string& word)
    {
        std::vector<std::string> result;

        size_t cSuggestions;
        char** suggestions = enchant_dict_suggest(_dict, word.c_str(), word.size(), &cSuggestions);

        if(suggestions != NULL){
            result.insert(result.begin(), suggestions, suggestions+cSuggestions);
        }

        FreeStringList(suggestions);
        
        return result;
    }

    std::vector<std::string> GetSuggestions(const std::string& s)
    {
        return GetSuggestionsFromWord("helo");
    }
};

#endif
