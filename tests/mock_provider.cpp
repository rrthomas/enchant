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

#include <glib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include "mock_provider.h"

static EnchantProviderDict *
mock_provider_request_dict(EnchantProvider *, const char *const)
{
    return NULL;
}

static void
mock_provider_dispose_dict(EnchantProvider *me, EnchantProviderDict *dict)
{
}

static const char *
mock_provider_identify (EnchantProvider *)
{
	return "mock";
}

static const char *
mock_provider_describe (EnchantProvider *)
{
	return "Mock Provider";
}

static char **
mock_provider_list_dicts (EnchantProvider *me, size_t *out_n_dicts)
{
    *out_n_dicts = 0;
    return NULL;
}

static ConfigureHook _hook;


extern "C" {

void
set_configure(ConfigureHook hook){
    _hook = hook;
}


EnchantProvider * 
init_enchant_provider(void)
{
#ifndef ENCHANT_TEST_NULL_IDENTIFY
    bool hasIdentify = true;
#else
    bool hasIdentify = false;
#endif
#ifndef ENCHANT_TEST_NULL_DESCRIBE
    bool hasDescribe = true;
#else
    bool hasDescribe = false;
#endif

#ifdef ENCHANT_TEST_NULL_PROVIDER
    return NULL;
#endif

    EnchantProvider *provider;
	
    provider = enchant_provider_new();
    provider->request_dict = mock_provider_request_dict; // this is required or module won't load
    provider->dispose_dict = mock_provider_dispose_dict;
    provider->identify = hasIdentify ? mock_provider_identify : NULL; // this is required or module won't load
    provider->describe = hasDescribe ? mock_provider_describe : NULL; // this is required or module won't load
    provider->list_dicts = mock_provider_list_dicts;
    provider->dictionary_exists = NULL;

    return provider;
}

void
configure_enchant_provider(EnchantProvider * me, const char *dir_name)
{
    if(_hook){
        _hook(me, dir_name);
    }
}

}
