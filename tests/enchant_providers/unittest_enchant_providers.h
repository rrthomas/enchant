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

#include "enchant.h"
#include "enchant-provider.h"
#include <glib.h>

EnchantProvider* GetProviderForTests();
char* GetErrorMessage(EnchantProvider* provider);

struct Provider_TestFixture
{
    EnchantProvider* _provider;

    //Setup
    Provider_TestFixture()
    { 
        _provider = GetProviderForTests();
    }
    //Teardown
    ~Provider_TestFixture()
    {
    }

    std::string Convert(const std::wstring & ws)
    {
        gchar* str = g_utf16_to_utf8((gunichar2*)ws.c_str(), (glong)ws.length(), NULL, NULL, NULL);
        std::string s(str);
        g_free(str);
        return s;
    }

    std::wstring Convert(const std::string & s)
    {
        gunichar2* str = g_utf8_to_utf16(s.c_str(), (glong)s.length(), NULL, NULL, NULL);
        std::wstring ws((wchar_t*)str);
        g_free(str);
        return ws;
    }

    EnchantDict* GetFirstAvailableDictionary()
    {
        EnchantDict* dict=NULL;        

        // get the first dictionary listed as being available
        if(_provider->list_dicts && _provider->request_dict)
        {
                size_t n_dicts;

	    	char ** dicts = (*_provider->list_dicts) (_provider, &n_dicts);
                for (size_t i = 0; i < n_dicts; i++)
                {
                        dict = (*_provider->request_dict) (_provider, dicts[i]);
                        break;
                }
                g_strfreev (dicts);
        }
        return dict;
    }

    EnchantDict* GetDictionary(const char* language)
    {
        if(_provider->request_dict)
        {
            return (*_provider->request_dict) (_provider, language);
        }
        return NULL;
    }

    virtual void ReleaseDictionary(EnchantDict* dict)
    {
        if (dict && _provider->dispose_dict)
        {
            _provider->dispose_dict(_provider, dict);
        }
    }
};
