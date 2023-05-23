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

#include "config.h"
#include <stdio.h>
#include <UnitTest++/UnitTest++.h>
#include <enchant.h>
#include <enchant-provider.h>
#include <glib.h>
#include <gmodule.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "unittest_enchant_providers.h"

int Test(char* path);
int TestProvider(char* filename);
int TestProvidersInDirectory(char * dir_name);

typedef EnchantProvider *(*EnchantProviderInitFunc) (void);
typedef void             (*EnchantPreConfigureFunc) (EnchantProvider * provider, const char * module_dir);

// from enchant.c: we need this so that providers can set errors.
struct str_enchant_broker
{
	GSList *provider_list;	/* list of all of the spelling backend providers */
	GHashTable *dict_map;		/* map of language tag -> dictionary */
	GHashTable *provider_ordering; /* map of language tag -> provider order */

	gchar * error;
};

// comes with a list of directories or providers
int main(int argc, char* argv[])
{
#ifndef ENABLE_RELOCATABLE
    fprintf(stderr, "You must configure with --enable-relocatable to be able to run the tests\n");
    return 1;
#endif
    int result = 0;
    for(int i=1; i < argc; ++i)
    {
        int resultT = Test(argv[i]);
        if(resultT != 0)
        {
            result = resultT;
        }
    }

    if(argc == 1)
    {
        char* current_dir = g_get_current_dir();
        result = TestProvidersInDirectory(current_dir);
        g_free(current_dir);
    }

    return result;
}

EnchantProvider* g_provider;
EnchantProvider* GetProviderForTests()
{
    return g_provider;
}

char* GetErrorMessage(EnchantProvider* provider)
{
    return provider->owner->error;
}

//path is provider filename or directory containing providers
int Test(char* path)
{
    assert(path);
    if (g_file_test (path, (GFileTest)(G_FILE_TEST_IS_DIR))) 
	{
        return TestProvidersInDirectory(path);
    }
    else
    {
        return TestProvider(path);
    }
}

int TestProvidersInDirectory(char * dir_name)
{
    GDir *dir = g_dir_open (dir_name, 0, NULL);
    if (!dir) 
        return 0;
	
    size_t g_module_suffix_len = strlen (G_MODULE_SUFFIX);

    int result = 0;
    const char *dir_entry;
    while ((dir_entry = g_dir_read_name (dir)) != NULL)
        {
#define PROVIDER_PREFIX "enchant_"
            size_t entry_len = strlen (dir_entry);
            if ((entry_len > g_module_suffix_len) &&
                !strncmp(dir_entry, PROVIDER_PREFIX, sizeof(PROVIDER_PREFIX)) &&
                !strcmp(dir_entry + (entry_len - g_module_suffix_len), G_MODULE_SUFFIX))
                {
                    char *filename = g_build_filename (dir_name, dir_entry, NULL);
                    int resultT = Test(filename);
                    if(resultT != 0)
                        result = resultT;
                    g_free (filename);
                }
        }
    
    g_dir_close (dir);
    return result;
}

int TestProvider(char* filename)
{
    assert(g_provider == NULL);
    int result = 0;

	EnchantProviderInitFunc init_func;
	EnchantPreConfigureFunc conf_func;

    GModule* module = g_module_open (filename, (GModuleFlags) 0);
	if (module) 
		{
			if (g_module_symbol(module, "init_enchant_provider", (gpointer *) (&init_func))
				&& init_func)
				{
					g_provider = init_func ();
				}

			/* optional entry point to allow modules to look for associated files
			 */
			if (g_provider && 
                g_module_symbol(module, "configure_enchant_provider", (gpointer *) (&conf_func))
				&& conf_func)
				{
                    char* dir_name = g_path_get_dirname(filename);
					conf_func (g_provider, dir_name);
                    g_free(dir_name);
				}
		} 
	else 
		{
			g_warning ("Could not load provider: %s\n", g_module_error());
		}

	if (g_provider)
	{
        EnchantBroker broker; // just so we have someplace to put errors
        broker.error=NULL;

        g_provider->enchant_private_data = (void *) module;
		g_provider->owner = &broker;
        printf("\nRunning tests on %s\n", filename);
        result = UnitTest::RunAllTests();
		if(g_provider->dispose)
			g_provider->dispose(g_provider);

        g_provider = NULL;
        free(broker.error);
	}

    if(module){
        g_module_close(module);
    }
    return result;
}
