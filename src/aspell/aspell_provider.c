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
 * Boston, MA 02111-1307, USA.
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

#ifdef HAVE_PSPELL_H
#include <pspell/pspell.h>
#else
#include <aspell.h>

/* to allow this to build on places that lack pspell.h */

#define PspellManager AspellSpeller
#define PspellWordList AspellWordList
#define PspellStringEmulation AspellStringEnumeration
#define PspellCanHaveError   AspellCanHaveError 
#define PspellConfig AspellConfig

#define new_pspell_config new_aspell_config
#define delete_pspell_config delete_aspell_config
#define pspell_config_replace aspell_config_replace

#define new_pspell_manager new_aspell_speller
#define to_pspell_manager to_aspell_speller
#define delete_pspell_manager delete_aspell_speller
#define pspell_manager_error_number aspell_speller_error_number
#define pspell_manager_error_message aspell_speller_error_message
#define pspell_manager_save_all_word_lists aspell_speller_save_all_word_lists
#define pspell_manager_check aspell_speller_check
#define pspell_manager_add_to_personal aspell_speller_add_to_personal
#define pspell_manager_add_to_session aspell_speller_add_to_session
#define pspell_manager_suggest aspell_speller_suggest
#define pspell_manager_store_replacement aspell_speller_store_replacement

#define pspell_word_list_elements aspell_word_list_elements
#define pspell_word_list_size aspell_word_list_size

#define pspell_string_emulation_next    aspell_string_enumeration_next
#define delete_pspell_string_emulation  delete_aspell_string_enumeration

#define pspell_error_message    aspell_error_message
#define pspell_error_number     aspell_error_number

#endif

#include "enchant.h"
#include "enchant-provider.h"

ENCHANT_PLUGIN_DECLARE("Aspell")

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT(void)
     configure_enchant_provider(EnchantProvider * me, const char *dir_name);

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
     init_enchant_provider (void);

#ifdef __cplusplus
}
#endif

static int
aspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	PspellManager *manager;
	int val;
	char *normalizedWord;

	manager = (PspellManager *) me->user_data;

	normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	val = pspell_manager_check (manager, normalizedWord, strlen(normalizedWord));
	g_free(normalizedWord);

	if (val == 0)
		return 1;
	else if (val > 0)
		return 0;
	else {
		enchant_dict_set_error (me, pspell_manager_error_message (manager));
		return -1;
	}
}

static char **
aspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	PspellManager *manager;
	
	const PspellWordList *word_list;
	PspellStringEmulation *suggestions;
	char *normalizedWord;
	
	char **sugg_arr = NULL;
	size_t n_suggestions, i;
	const char *sugg;
	
	manager = (PspellManager *) me->user_data;
	
	normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	word_list = pspell_manager_suggest (manager, normalizedWord, strlen(normalizedWord));
	g_free(normalizedWord);

	if (word_list)
		{
			suggestions = pspell_word_list_elements (word_list);
			if (suggestions)
				{
					n_suggestions = pspell_word_list_size (word_list);
					*out_n_suggs = n_suggestions;
					
					if (n_suggestions)
						{
							sugg_arr = g_new0 (char *, n_suggestions + 1);
							
							for (i = 0; i < n_suggestions; i++)
								{
									sugg = pspell_string_emulation_next (suggestions);
									if (sugg)
										sugg_arr[i] = g_strdup (sugg);
								}
						}
					delete_pspell_string_emulation (suggestions);
				}
		}
	
	return sugg_arr;
}

static void
aspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_add_to_personal (manager, word, len);
	pspell_manager_save_all_word_lists (manager);
}

static void
aspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_add_to_session (manager, word, len);
}

static void
aspell_dict_store_replacement (EnchantDict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_store_replacement (manager, mis, mis_len,
					  cor, cor_len);
	pspell_manager_save_all_word_lists (manager);
}

static EnchantDict *
aspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	PspellManager *manager;
	PspellConfig *spell_config;
	PspellCanHaveError *spell_error;
	
	spell_config = new_pspell_config ();
	pspell_config_replace (spell_config, "language-tag", tag);
	pspell_config_replace (spell_config, "encoding", "utf-8");
	
	spell_error = new_pspell_manager (spell_config);
	delete_pspell_config (spell_config);
	
	if (pspell_error_number (spell_error) != 0)
		{
			return NULL;
		}
	
	manager = to_pspell_manager (spell_error);
	
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
	PspellManager *manager;
	
	manager = (PspellManager *) dict->user_data;
	delete_pspell_manager (manager);
	
	g_free (dict);
}

#if ASPELL_0_50_0
static char ** 
aspell_provider_list_dicts (EnchantProvider * me, 
			    size_t * out_n_dicts)
{
	PspellConfig * spell_config;
	AspellDictInfoList * dlist;
	AspellDictInfoEnumeration * dels;
	const AspellDictInfo * entry;
	char ** out_list = NULL;
	
	spell_config = new_pspell_config ();

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
	
	delete_pspell_config (spell_config);
	
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

#ifdef __cplusplus
extern "C" {
#endif

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

#if ASPELL_0_50_0
	provider->list_dicts = aspell_provider_list_dicts;
#else
#  ifdef __GNUC__
#    warning "You're using an ancient version of Aspell. Some things won't work properly."
#  endif
#endif
	provider->free_string_list = aspell_provider_free_string_list;

	return provider;
}


#if defined(_WIN32)

static WCHAR* GetDirectoryOfThisLibrary()
{
	WCHAR dll_path[MAX_PATH];
    gchar* utf8_dll_path;
    gchar* utf8_prefix;
    gunichar2* utf16_prefix;

    if(!GetModuleFileName(s_hModule,dll_path,MAX_PATH))
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

    h = LoadLibrary(wszFullLibraryPath);

    g_free(wszFullLibraryPath);
    return h;
}

static WCHAR* GetRegistryValue(HKEY baseKey, const WCHAR * uKeyName, const WCHAR * uKey)
{
  	HKEY hKey;
	unsigned long lType;	
	DWORD dwSize;
	WCHAR* wszValue = NULL;

	if(RegOpenKeyEx(baseKey, uKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			/* Determine size of string */
			if(RegQueryValueEx( hKey, uKey, NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS)
				{
					wszValue = g_new0(WCHAR, dwSize + 1);
					RegQueryValueEx(hKey, uKey, NULL, &lType, (LPBYTE) wszValue, &dwSize);
				}
		}

	return wszValue;
}
#endif

void configure_enchant_provider(EnchantProvider * me, const char *dir_name)
{
#if defined(_WIN32)
    const WCHAR* aspell_module_name = L"aspell-15.dll";
    HMODULE aspell_module = NULL;
    char* szModule;

    /* first try load from registry path */
   	szModule = enchant_get_registry_value ("Aspell", "Module");
    if(szModule)
    {
        WCHAR* wszModule;

	    wszModule = g_utf8_to_utf16 (szModule, -1, NULL, NULL, NULL);
        aspell_module = LoadLibrary(wszModule);
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
            aspell_module = LoadLibrary(aspell_module_name);
        }

    if (aspell_module == NULL) 
        {
            /* we can't seem to load aspell. Avoid late binding problems later */
            g_warning("Unable to load library aspell-15.dll.");
            me->request_dict = NULL;
            me->dispose_dict = NULL;
            me->list_dicts = NULL;
        }
#endif

}


#ifdef __cplusplus
}
#endif
