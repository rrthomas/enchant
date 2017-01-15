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

#ifndef __ENCHANTBROKERTESTFIXTURE
#define __ENCHANTBROKERTESTFIXTURE

#if defined(_MSC_VER)
#pragma once
#pragma warning(push)
#pragma warning(disable:4505) //unreferenced local function has been removed
#pragma warning(disable: 4996) //The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name.
#endif

#include "EnchantTestFixture.h"
#include "mock_provider/mock_provider.h"
#include <stack>
#include <stdio.h>
#include <direct.h>

//////////////////////////////////////
// Mock provider functions
static void
MockProviderDispose(EnchantProvider *me)
{
    g_free(me);
}

static int
MockEnGbAndQaaProviderDictionaryExists (EnchantProvider *,
				     const char *const tag)
{
    return (strcmp(tag, "en_GB")==0 || strcmp(tag, "qaa") ==0);
}


static EnchantDict*
MockEnGbAndQaaProviderRequestDictionary(EnchantProvider * me, const char *tag)
{
  	EnchantDict *dict = NULL;

    if(MockEnGbAndQaaProviderDictionaryExists(me, tag)){
	    dict = g_new0 (EnchantDict, 1);
    }
	return dict;
}

static void
MockProviderDisposeDictionary (EnchantProvider *, EnchantDict * dict)
{
    g_free(dict);
}

static const char *
MockProviderIdentify (EnchantProvider *)
{
	return "mock";
}

static const char *
MockProviderDescribe (EnchantProvider *)
{
	return "Mock Provider";
}

static char ** 
MockEnGbAndQaaProviderListDictionaries (EnchantProvider *, 
			      size_t * out_n_dicts)
{
    *out_n_dicts = 2;
    char** out_list = g_new0 (char *, *out_n_dicts + 1);
    out_list[0] = g_strdup ("en_GB");
    out_list[1] = g_strdup ("qaa");

    return out_list;
}

static char ** 
MockEnGbProviderListDictionaries (EnchantProvider *, 
			      size_t * out_n_dicts)
{
    *out_n_dicts = 1;
    char** out_list = g_new0 (char *, *out_n_dicts + 1);
    out_list[0] = g_strdup ("en_GB");

    return out_list;
}

static void
MockProviderFreeStringList (EnchantProvider *, char **str_list)
{
	g_strfreev (str_list);
}

typedef void (*SET_CONFIGURE)(ConfigureHook);

struct EnchantBrokerTestFixture : EnchantTestFixture
{
    EnchantBroker* _broker;
    HMODULE hModule, hModule2;

    //Setup
    EnchantBrokerTestFixture(ConfigureHook userConfiguration=NULL, 
                             ConfigureHook user2Configuration=NULL,
                             bool includeNullProviders = false)
    {
        userMockProviderConfiguration = userConfiguration;
        userMockProvider2Configuration = user2Configuration;

#if _WIN32
        CopyProvider("libenchant_mock_provider", "libenchant_mock_provider");
        hModule = LoadLibrary(L"lib\\enchant\\libenchant_mock_provider.dll");
        if(hModule!=NULL){
            SET_CONFIGURE sc = (SET_CONFIGURE) GetProcAddress(hModule, "set_configure");
            (sc)(ConfigureMockProvider);
        }

        hModule2 = NULL;
        if(user2Configuration != NULL){
            CopyProvider("libenchant_mock_provider", "libenchant_mock_provider2");
            hModule2 = LoadLibrary(L"lib\\enchant\\libenchant_mock_provider2.dll");
            if(hModule2!=NULL){
                SET_CONFIGURE sc = (SET_CONFIGURE) GetProcAddress(hModule2, "set_configure");
                (sc)(ConfigureMockProvider2);
            }
        }
#else
        // What to do?
#endif

        if(includeNullProviders){
            CopyProvider("libenchant_mock_provider", "null_provider");
            CopyProvider("libenchant_mock_provider", "null_identify");
            CopyProvider("libenchant_mock_provider", "null_describe");
            CopyProvider("libenchant", "libenchant"); //not a provider
        }

        SetUserRegistryConfigDir(GetTempUserEnchantDir());
        InitializeBroker();
    }

    //Teardown
    ~EnchantBrokerTestFixture(){
#if _WIN32
        if(hModule!=NULL){
          FreeLibrary(hModule);
        }
        if(hModule2!=NULL){
          FreeLibrary(hModule2);
        }
#endif

        if(_broker){
            enchant_broker_free (_broker);
        }
        while(!pwlFilenames.empty())
        {
            DeleteFile(pwlFilenames.top());
            pwlFilenames.pop();
        }
    }

    void InitializeBroker()
    {
        _broker = enchant_broker_init ();
    }

    void CopyProvider(const std::string& sourceProviderName, const std::string& destinationProviderName)
    {
        std::string sourceName = sourceProviderName +"."+ G_MODULE_SUFFIX;
        std::string destinationName = destinationProviderName + "." +G_MODULE_SUFFIX;

        std::string destinationDir = AddToPath(AddToPath(GetDirectoryOfThisModule(), "lib"),"enchant");

        CreateDirectory(destinationDir);

        std::string destinationPath = AddToPath(destinationDir, destinationName);

        gchar * contents;
        gsize length;
        if(g_file_get_contents(AddToPath(GetDirectoryOfThisModule(), sourceName).c_str(),
                            &contents, 
                            &length, 
                            NULL)){
            g_file_set_contents(destinationPath.c_str(),
                                contents,
                                length,
                                NULL);
        }
    }


    EnchantProvider* GetMockProvider(){
        return mock_provider;
    }

    void SetErrorOnMockProvider(const std::string& error)
    {
        EnchantProvider* provider = GetMockProvider();
        if(provider != NULL)
        {
           enchant_provider_set_error(provider, error.c_str());
        }
    }

    EnchantDict* RequestDictionary(const std::string& tag){
        return enchant_broker_request_dict(_broker, tag.c_str());
    }

    EnchantDict* RequestPersonalDictionary()
    {
        std::string pwlFileName = GetTemporaryFilename("epwl");
        CreateFile(pwlFileName);
        pwlFilenames.push(pwlFileName);
        return enchant_broker_request_pwl_dict(_broker, pwlFileName.c_str());
    }

    std::string GetLastPersonalDictionaryFileName()
    {
      return pwlFilenames.top();
    }


    void FreeDictionary(EnchantDict* dictionary){
        if(dictionary != NULL){
            enchant_broker_free_dict(_broker, dictionary);
        }
    }

    private: 
     std::stack<std::string> pwlFilenames;
     static EnchantProvider * mock_provider;
     static ConfigureHook userMockProviderConfiguration;
     static ConfigureHook userMockProvider2Configuration;
     static void ConfigureMockProvider (EnchantProvider * me, const char * dir_name)
    {
        mock_provider = me;
        if(userMockProviderConfiguration){
            userMockProviderConfiguration(me, dir_name);
        }
    }

    static void ConfigureMockProvider2 (EnchantProvider * me, const char * dir_name)
    {
        mock_provider = me;
        if(userMockProvider2Configuration){
            userMockProvider2Configuration(me, dir_name);
        }
    }
};

struct DictionaryDescription 
{
    std::string LanguageTag;
    std::string Name;
    std::string Description;
    std::string DllFile;
    DictionaryDescription()
    {}
    DictionaryDescription(const char* const language_tag,
                          const char * const provider_name,
  	                      const char * const provider_desc,
                          const char * const provider_file):
        LanguageTag(language_tag),
        Name(provider_name), 
        Description(provider_desc), 
        DllFile(provider_file)
    {}

    DictionaryDescription(const DictionaryDescription& d):
        LanguageTag(d.LanguageTag),
        Name(d.Name), 
        Description(d.Description), 
        DllFile(d.DllFile)
    {}

    bool DataIsComplete()
    {
        return (LanguageTag.length() &&
                Name.length() &&
                Description.length() &&
                DllFile.length());
    }
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
