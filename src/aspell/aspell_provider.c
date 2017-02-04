/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003,2004 Dom Lachowicz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <aspell.h>

#include "enchant.h"
#include "enchant-provider.h"

ENCHANT_PLUGIN_DECLARE("Aspell")

ENCHANT_MODULE_EXPORT(void)
     configure_enchant_provider(EnchantProvider * me, const char *dir_name);

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
     init_enchant_provider (void);

static int
aspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	AspellSpeller *manager;
	int val;
	char *normalizedWord;

	manager = (AspellSpeller *) me->user_data;

	normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	val = aspell_speller_check (manager, normalizedWord, strlen(normalizedWord));
	g_free(normalizedWord);

	if (val == 0)
		return 1;
	else if (val > 0)
		return 0;
	else {
		enchant_dict_set_error (me, aspell_speller_error_message (manager));
		return -1;
	}
}

static char **
aspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	AspellSpeller *manager;
	
	const AspellWordList *word_list;
	AspellStringEnumeration *suggestions;
	char *normalizedWord;
	
	char **sugg_arr = NULL;
	size_t n_suggestions, i;
	const char *sugg;
	
	manager = (AspellSpeller *) me->user_data;
	
	normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	word_list = aspell_speller_suggest (manager, normalizedWord, strlen(normalizedWord));
	g_free(normalizedWord);

	if (word_list)
		{
			suggestions = aspell_word_list_elements (word_list);
			if (suggestions)
				{
					n_suggestions = aspell_word_list_size (word_list);
					*out_n_suggs = n_suggestions;
					
					if (n_suggestions)
						{
							sugg_arr = g_new0 (char *, n_suggestions + 1);
							
							for (i = 0; i < n_suggestions; i++)
								{
									sugg = aspell_string_enumeration_next (suggestions);
									if (sugg)
										sugg_arr[i] = g_strdup (sugg);
								}
						}
					delete_aspell_string_enumeration (suggestions);
				}
		}
	
	return sugg_arr;
}

static void
aspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	AspellSpeller *manager;
	
	manager = (AspellSpeller *) me->user_data;
	aspell_speller_add_to_personal (manager, word, len);
	aspell_speller_save_all_word_lists (manager);
}

static void
aspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	AspellSpeller *manager;
	
	manager = (AspellSpeller *) me->user_data;
	aspell_speller_add_to_session (manager, word, len);
}

static void
aspell_dict_store_replacement (EnchantDict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
	AspellSpeller *manager;
	
	manager = (AspellSpeller *) me->user_data;
	aspell_speller_store_replacement (manager, mis, mis_len,
					  cor, cor_len);
	aspell_speller_save_all_word_lists (manager);
}

static EnchantDict *
aspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	AspellSpeller *manager;
	AspellConfig *spell_config;
	AspellCanHaveError *spell_error;
	
	spell_config = new_aspell_config ();
	aspell_config_replace (spell_config, "language-tag", tag);
	aspell_config_replace (spell_config, "encoding", "utf-8");
	
	spell_error = new_aspell_speller (spell_config);
	delete_aspell_config (spell_config);
	
	if (aspell_error_number (spell_error) != 0)
		{
			return NULL;
		}
	
	manager = to_aspell_speller (spell_error);
	
	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) manager;
	dict->check = aspell_dict_check;
	dict->suggest = aspell_dict_suggest;
	dict->add_to_personal = aspell_dict_add_to_personal;
	dict->add_to_session = aspell_dict_add_to_session;
	dict->store_replacement = aspell_dict_store_replacement;
	
	return dict;
}

static void
aspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	AspellSpeller *manager;
	
	manager = (AspellSpeller *) dict->user_data;
	delete_aspell_speller (manager);
	
	g_free (dict);
}

#if ASPELL_0_50_0
static char ** 
aspell_provider_list_dicts (EnchantProvider * me, 
			    size_t * out_n_dicts)
{
	AspellConfig * spell_config;
	AspellDictInfoList * dlist;
	AspellDictInfoEnumeration * dels;
	const AspellDictInfo * entry;
	char ** out_list = NULL;
	
	spell_config = new_aspell_config ();

	dlist = get_aspell_dict_info_list (spell_config);

	*out_n_dicts = 0;
	dels = aspell_dict_info_list_elements (dlist);

	/* TODO: Use aspell_dict_info_list_size() once it is implemented and returns non-zero. */
	while ( (entry = aspell_dict_info_enumeration_next(dels)) != 0)
		(*out_n_dicts)++;

	if (*out_n_dicts) {
		size_t i;		

		out_list = g_new0 (char *, *out_n_dicts + 1);
		dels = aspell_dict_info_list_elements (dlist);
		
		for (i = 0; i < *out_n_dicts; i++) {
			entry = aspell_dict_info_enumeration_next (dels);			
			/* XXX: should this be entry->code or entry->name ? */
			out_list[i] = g_strdup (entry->code);
		}
		
		delete_aspell_dict_info_enumeration (dels);
	}
	
	delete_aspell_config (spell_config);
	
	return out_list;
}
#endif

static void
aspell_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static void
aspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
aspell_provider_identify (EnchantProvider * me)
{
	return "aspell";
}

static const char *
aspell_provider_describe (EnchantProvider * me)
{
	return "Aspell Provider";
}

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = aspell_provider_dispose;
	provider->request_dict = aspell_provider_request_dict;
	provider->dispose_dict = aspell_provider_dispose_dict;
	provider->identify = aspell_provider_identify;
	provider->describe = aspell_provider_describe;
	provider->list_dicts = aspell_provider_list_dicts;
	provider->free_string_list = aspell_provider_free_string_list;

	return provider;
}


#if defined(_WIN32)

static WCHAR* GetDirectoryOfThisLibrary(void)
{
	WCHAR dll_path[MAX_PATH];
    gchar* utf8_dll_path;
    gchar* utf8_prefix;
    gunichar2* utf16_prefix;

    if(!GetModuleFileNameW(s_hModule,dll_path,MAX_PATH))
        { /* unable to determine filename of this library */
            return NULL;
        }
	utf8_dll_path = g_utf16_to_utf8 (dll_path, -1, NULL, NULL, NULL);
	utf8_prefix = g_path_get_dirname(utf8_dll_path);
	g_free(utf8_dll_path);

    utf16_prefix = g_utf8_to_utf16 (utf8_prefix, -1, NULL, NULL, NULL);
	g_free(utf8_prefix);

    return utf16_prefix;
}

static HMODULE LoadLibraryFromPath(const WCHAR* path, const WCHAR* libraryName)
{
    HMODULE h;
	WCHAR* wszFullLibraryPath;
    size_t fullLibraryPathLen;

    fullLibraryPathLen = wcslen(path) +  1 /* '\\' */+ wcslen(libraryName);
    wszFullLibraryPath = g_new0(WCHAR, fullLibraryPathLen + 1);

    wcscpy(wszFullLibraryPath, path);
    wcscat(wszFullLibraryPath, L"\\");
    wcscat(wszFullLibraryPath, libraryName);

    h = LoadLibraryW(wszFullLibraryPath);

    g_free(wszFullLibraryPath);
    return h;
}

static WCHAR* GetRegistryValue(HKEY baseKey, const WCHAR * uKeyName, const WCHAR * uKey)
{
  	HKEY hKey;
	unsigned long lType;	
	DWORD dwSize;
	WCHAR* wszValue = NULL;

	if(RegOpenKeyExW(baseKey, uKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			/* Determine size of string */
			if(RegQueryValueExW( hKey, uKey, NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS)
				{
					wszValue = g_new0(WCHAR, dwSize + 1);
					RegQueryValueExW(hKey, uKey, NULL, &lType, (LPBYTE) wszValue, &dwSize);
				}
		}

	return wszValue;
}
#endif

#if defined(_WIN32)
gboolean load_library(EnchantProvider * me, const char *dir_name, const WCHAR *aspell_module_name)
{
    HMODULE aspell_module = NULL;
    char* szModule;

    /* first try load from registry path */
   	szModule = enchant_get_registry_value ("Aspell", "Module");
    if(szModule)
    {
        WCHAR* wszModule;

	    wszModule = g_utf8_to_utf16 (szModule, -1, NULL, NULL, NULL);
        aspell_module = LoadLibraryW(wszModule);
        g_free(wszModule);
    }

    if (aspell_module == NULL)
        {
            /* next try load from aspell registry path */
            WCHAR* wszDirectory = GetRegistryValue (HKEY_LOCAL_MACHINE, L"Software\\Aspell", L"Path");
            if(wszDirectory)
                {
                    aspell_module = LoadLibraryFromPath(wszDirectory, aspell_module_name);
                    g_free(wszDirectory);
                }
        }

    if (aspell_module == NULL)
        {
            /* then try from same directory as provider */
            WCHAR* wszDirectory = GetDirectoryOfThisLibrary();
            if(wszDirectory)
                {
                    aspell_module = LoadLibraryFromPath(wszDirectory, aspell_module_name);
                    g_free(wszDirectory);
                }
        }

    if (aspell_module == NULL) 
        {
            /* then try default lookup */
            aspell_module = LoadLibraryW(aspell_module_name);
        }

    if (aspell_module == NULL) 
        {
	        return FALSE;
        }
    return TRUE;
}
#endif

void configure_enchant_provider(EnchantProvider * me, const char *dir_name)
{
#if defined(_WIN32)
	if(!load_library(me, dir_name, L"aspell-15.dll") &&
       !load_library(me, dir_name, L"libaspell-15.dll"))
    {
            /* we can't seem to load aspell. Avoid late binding problems later */
            g_warning("Unable to load library aspell-15.dll.");
            me->request_dict = NULL;
            me->dispose_dict = NULL;
            me->list_dicts = NULL;
    }
#endif
}
