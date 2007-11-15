/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003, 2004 Dom Lachowicz
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
#include <gmodule.h>
#include <locale.h>

#include "enchant.h"
#include "enchant-provider.h"
#include "pwl.h"

#ifdef XP_TARGET_COCOA
#import "enchant_cocoa.h"
#endif

#ifdef XP_TARGET_COCOA
#define ENCHANT_USER_PATH_EXTENSION "Library", "Application Support", "Enchant"
#else
#define ENCHANT_USER_PATH_EXTENSION ".enchant"
#endif

#ifdef ENABLE_BINRELOC
#include "prefix.h"
#endif

ENCHANT_PLUGIN_DECLARE("Enchant")

/********************************************************************************/
/********************************************************************************/

struct str_enchant_broker
{
	GSList *provider_list;	/* list of all of the spelling backend providers */
	GHashTable *dict_map;		/* map of language tag -> dictionary */
	GHashTable *provider_ordering; /* map of language tag -> provider order */

	gchar * error;
};

typedef struct str_enchant_session
{
	GHashTable *session;
	EnchantPWL *personal;

	char * personal_filename;
	char * language_tag;

	char * error;

	gboolean is_pwl;

	EnchantProvider * provider;
} EnchantSession;

typedef EnchantProvider *(*EnchantProviderInitFunc) (void);
typedef void             (*EnchantPreConfigureFunc) (EnchantProvider * provider, const char * module_dir);

/********************************************************************************/
/********************************************************************************/

static void
_enchant_ensure_private_datadir (void)
{
	/* test if ~/.enchant exists  */
	char * home_dir;

	home_dir = enchant_get_user_home_dir ();
		if (home_dir) {
				char * enchant_path;
		
		enchant_path = g_build_filename (home_dir, ENCHANT_USER_PATH_EXTENSION, NULL);
		if (enchant_path && !g_file_test (enchant_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) 
			{
				(void)g_remove (enchant_path);
				g_mkdir (enchant_path, 0700);                        
			}
		
		g_free (enchant_path);
		g_free (home_dir);
	}
}

static char *
enchant_get_module_dir (void)
{
	char * module_dir = NULL;
	char * prefix = NULL;

#ifdef XP_TARGET_COCOA
	return g_strdup ([[EnchantResourceProvider instance] moduleFolder]);
#endif

	/* Look for explicitly set registry values */
	module_dir = enchant_get_registry_value ("Config", "Module_Dir");
	if (module_dir)
				return module_dir;

	/* Dynamically locate library and search for modules relative to it. */
	prefix = enchant_get_prefix_dir();
	if(prefix)
		{
			module_dir = g_build_filename(prefix,"lib","enchant",NULL);
			g_free(prefix);
			return module_dir;
		}

#if defined(ENCHANT_GLOBAL_MODULE_DIR)
	return g_strdup (ENCHANT_GLOBAL_MODULE_DIR);
#else
	return NULL;
#endif
}

static char *
enchant_get_conf_dir (void)
{
#ifdef XP_TARGET_COCOA
	return g_strdup ([[EnchantResourceProvider instance] configFolder]);
#endif
	char * ordering_dir = NULL, * prefix = NULL;;

	/* Look for explicitly set registry values */
	ordering_dir = enchant_get_registry_value ("Config", "Data_Dir");
	if (ordering_dir)
		return ordering_dir;

	/* Dynamically locate library and search for files relative to it. */
	prefix = enchant_get_prefix_dir();
	if(prefix)
		{
			ordering_dir = g_build_filename(prefix,"share","enchant",NULL);
			g_free(prefix);
			return ordering_dir;
		}

#if defined(ENCHANT_GLOBAL_ORDERING)
	return g_strdup (ENCHANT_GLOBAL_ORDERING);
#else
	return NULL;
#endif
}

/*
 * Returns: the value if it exists and is not an empty string ("") or %null otherwise. Must be free'd.
 */
static char *
enchant_get_registry_value_ex (int current_user, const char * const prefix, const char * const key)
{
#ifndef _WIN32
	/* TODO: GConf? KConfig? */
	return NULL;
#else
	HKEY hKey;
	HKEY baseKey;
	unsigned long lType;	
	DWORD dwSize;
	char* keyName;
	WCHAR* wszValue = NULL;
	char* szValue = NULL;
	gunichar2 * uKeyName;
	gunichar2 * uKey;

	if (current_user)
		baseKey = HKEY_CURRENT_USER;
	else
		baseKey = HKEY_LOCAL_MACHINE;

	keyName = g_strdup_printf("Software\\Enchant\\%s", prefix);
	uKeyName = g_utf8_to_utf16 (keyName, -1, NULL, NULL, NULL);
	uKey = g_utf8_to_utf16 (key, -1, NULL, NULL, NULL);

	if(RegOpenKeyEx(baseKey, uKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			// Determine size of string
			if(RegQueryValueEx( hKey, uKey, NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS)
				{
					wszValue = g_new0(WCHAR, dwSize + 1);
					RegQueryValueEx(hKey, uKey, NULL, &lType, (LPBYTE) wszValue, &dwSize);
				}
		}

	if(wszValue && *wszValue)
		szValue = g_utf16_to_utf8 (wszValue, -1, NULL, NULL, NULL);

	g_free(keyName);
	g_free(uKeyName);
	g_free(uKey);
	g_free(wszValue);

	return szValue;
#endif
}

/**
 * enchant_get_registry_value
 * @prefix: Your category, such as "Ispell" or "Myspell"
 * @key: The tag within your category that you're interested in
 *
 * Returns: the value if it exists and is not an empty string ("") or %null otherwise. Must be free'd.
 *
 * This API is private to the providers.
 */
ENCHANT_MODULE_EXPORT (char *)
enchant_get_registry_value (const char * const prefix, const char * const key)
{
	char *val;

	g_return_val_if_fail (prefix, NULL);
	g_return_val_if_fail (key, NULL);

	val = enchant_get_registry_value_ex(1, prefix, key);
	if(val == NULL) {
			val = enchant_get_registry_value_ex (0, prefix, key);
		}
	return val;
}

/**
 * enchant_get_user_home_dir
 *
 * Returns: the user's home directory, or %null. Returned value
 * must be free'd.
 *
 * This API is private to the providers.
 */
ENCHANT_MODULE_EXPORT (char *)
enchant_get_user_home_dir (void)
{
	const char* home_dir;

	home_dir = enchant_get_registry_value_ex (1, "Config", "Home_Dir");
	if (home_dir)
		return (char *)home_dir;

	home_dir = g_get_home_dir ();
	if (home_dir)
		return g_strdup (home_dir);
	return NULL;
}

/********************************************************************************/
/********************************************************************************/

static gchar*
enchant_modify_string_chars (gchar *str,
							 gssize len,
							 gchar (*function)(gchar))
{
	gchar* it, *end;

	g_return_val_if_fail (str != NULL, NULL);

	if (len < 0)
		len = strlen (str);

	end = str + len;

	for (it = str; it != end; ++it)
		*it = function (*it);

	return str;
}

static gchar*
enchant_ascii_strup (gchar *str,
					 gssize len)
{
	return enchant_modify_string_chars(str, len, g_ascii_toupper);
}

static gchar*
enchant_ascii_strdown (gchar *str,
						  gssize len)
{
	return enchant_modify_string_chars(str, len, g_ascii_tolower);
}

static char *
enchant_normalize_dictionary_tag (const char * const dict_tag)
{
	char * new_tag = g_strdup (dict_tag);
	char * needle;

	new_tag = g_strstrip (new_tag);

	/* strip off en_GB@euro */
	if ((needle = strchr (new_tag, '@')) != NULL)
		*needle = '\0';

	/* strip off en_GB.UTF-8 */
	if ((needle = strchr (new_tag, '.')) != NULL)
		*needle = '\0';

	/* turn en-GB into en_GB */
	if ((needle = strchr (new_tag, '-')) != NULL)
		*needle = '_';

	/* everything before first '_' is converted to lower case */
	if ((needle = strchr (new_tag, '_')) != NULL) {
			enchant_ascii_strdown(new_tag, needle - new_tag);
			++needle;
			/* everything after first '_' is converted to upper case */
			enchant_ascii_strup(needle, -1);
		}
	else {
			enchant_ascii_strdown(new_tag, -1);
		}

	return new_tag;
}

static char *
enchant_iso_639_from_tag (const char * const dict_tag)
{
	char * new_tag = g_strdup (dict_tag);
	char * needle;

	if ((needle = strchr (new_tag, '_')) != NULL)
		*needle = '\0';

	return new_tag;
}

static void
enchant_session_destroy (EnchantSession * session)
{
	g_hash_table_destroy (session->session);
	enchant_pwl_free (session->personal);
	g_free (session->personal_filename);
	g_free (session->language_tag);

	if (session->error)
		g_free (session->error);

	g_free (session);
}

static EnchantSession *
enchant_session_new_with_pwl (EnchantProvider * provider, const char * const pwl, const char * const lang,
				  gboolean fail_if_no_pwl)
{
	EnchantSession * session;
	EnchantPWL *personal = NULL;

	if (pwl)
		personal = enchant_pwl_init_with_file (pwl);

	if (personal == NULL) {
		if (fail_if_no_pwl)
			return NULL;
		else
			personal = enchant_pwl_init ();
	}
	

	session = g_new0 (EnchantSession, 1);
	session->session = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	session->personal = personal;
	session->provider = provider;
	session->language_tag = g_strdup (lang);
	session->personal_filename = g_strdup (pwl);
	
	return session;
}

static EnchantSession *
enchant_session_new (EnchantProvider *provider, const char * const lang)
{
	EnchantSession * session;
	char * home_dir, * dic = NULL, * filename;

	home_dir = enchant_get_user_home_dir ();
	if (home_dir) 
		{
			_enchant_ensure_private_datadir ();
			filename = g_strdup_printf ("%s.dic", lang);
			dic = g_build_filename (home_dir,
									ENCHANT_USER_PATH_EXTENSION,
									filename,
									NULL);
			g_free (filename);
			g_free (home_dir);
		}

	session = enchant_session_new_with_pwl (provider, dic, lang, FALSE);	
	
	if (dic)
		g_free (dic);
	
	return session;
}

static void
enchant_session_add (EnchantSession * session, const char * const word, size_t len)
{
	g_hash_table_insert (session->session, g_strndup (word, len), GINT_TO_POINTER(TRUE));
}

static void
enchant_session_add_personal (EnchantSession * session, const char * const word, size_t len)
{
	enchant_pwl_add(session->personal, word, len);
}

static gboolean
enchant_session_contains (EnchantSession * session, const char * const word, size_t len)
{
	gboolean result = FALSE;
	
	char * utf = g_strndup (word, len);
	
	if (g_hash_table_lookup (session->session, utf) ||
		enchant_pwl_check (session->personal, utf, len) == 0)
		result = TRUE;
	
	g_free (utf);

	return result;
}

static void
enchant_session_clear_error (EnchantSession * session)
{
	if (session->error) 
		{
			g_free (session->error);
			session->error = NULL;
		}
}

/********************************************************************************/
/********************************************************************************/

static void
enchant_provider_free_string_list (EnchantProvider * provider, char ** string_list)
{
	if (provider && provider->free_string_list)
		(*provider->free_string_list) (provider, string_list);
}

/**
 * enchant_dict_set_error
 * @dict: A non-null dictionary
 * @err: A non-null error message
 *
 * Sets the current runtime error to @err. This API is private to the
 * providers.
 */
ENCHANT_MODULE_EXPORT(void)
enchant_dict_set_error (EnchantDict * dict, const char * const err)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (err);
	
	session = (EnchantSession*)dict->enchant_private_data;

	enchant_session_clear_error (session);
	session->error = g_strdup (err);	
}

/**
 * enchant_dict_get_error
 * @dict: A non-null dictionary
 *
 * Returns a const char string or NULL describing the last exception.
 * WARNING: error is transient. It will likely be cleared as soon as 
 * the next dictionary operation is called
 *
 * Returns: an error message
 */
ENCHANT_MODULE_EXPORT(char *)
enchant_dict_get_error (EnchantDict * dict)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, NULL);
	
	session = (EnchantSession*)dict->enchant_private_data;

	return session->error;
}

/**
 * enchant_dict_check
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to check, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 * Will return an "incorrect" value if any of those pre-conditions
 * are not met.
 *
 * Returns: 0 if the word is correctly spelled, positive if not, negative if error
 */
ENCHANT_MODULE_EXPORT (int)
enchant_dict_check (EnchantDict * dict, const char *const word, ssize_t len)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, -1);
	g_return_val_if_fail (word, -1);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, -1);

	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);

	/* first, see if it's in our session */
	if (enchant_session_contains (session, word, len))
		return 0;

	if (dict->check)
		return (*dict->check) (dict, word, len);
	else if (session->is_pwl)
		return 1;

	return -1;
}

/**
 * enchant_dict_suggest
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to find suggestions for, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 * @out_n_suggs: The location to store the # of suggestions returned, or %null
 *
 * Will return an %null value if any of those pre-conditions
 * are not met.
 *
 * Returns: A %null terminated list of UTF-8 encoded suggestions, or %null
 */
ENCHANT_MODULE_EXPORT (char **)
enchant_dict_suggest (EnchantDict * dict, const char *const word,
			  ssize_t len, size_t * out_n_suggs)
{
	EnchantSession * session;
	size_t n_suggs = 0, n_dict_suggs = 0, n_pwl_suggs = 0;
	char **suggs, **dict_suggs = NULL, **pwl_suggs = NULL;

	g_return_val_if_fail (dict, NULL);
	g_return_val_if_fail (word, NULL);

	if (len < 0)
		len = strlen (word);

	g_return_val_if_fail (len, NULL);

	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);
	/* Check for suggestions from personal dictionary */
	if(session->personal)
			pwl_suggs = enchant_pwl_suggest(session->personal, word, len, &n_pwl_suggs);
		
	/* Check for suggestions from provider dictionary */
	if (dict->suggest) 
		{
			dict_suggs = (*dict->suggest) (dict, word, len,	
							&n_dict_suggs);
		}

	/* Clone suggestions if there are any */
	n_suggs = n_pwl_suggs + n_dict_suggs;
	if (n_suggs > 0)
		{
			size_t i, j, k;
			
			suggs = g_new0 (char *, n_suggs + 1);

			/* Copy over suggestions from dict */
			for(i = 0; i < n_dict_suggs; i++)
				suggs[i] = g_strdup (dict_suggs[i]);

			/* Copy over suggestions from pwl, except dupes */
			for(j = 0; j < n_pwl_suggs; j++) {
				int dupe = 0;
				for(k = 0; k < n_dict_suggs; k++) {
					if(strcmp(suggs[k],pwl_suggs[j])==0) {
						dupe = 1;
						--n_suggs;
						break;
					}
				}
				if(!dupe) {
					suggs[i] = g_strdup (pwl_suggs[j]);
					i++;
				}
			}
		}
	else 
		{
			suggs = NULL;
		}
	
	if(dict_suggs)
			enchant_provider_free_string_list (session->provider, dict_suggs);

	if(pwl_suggs)
			enchant_pwl_free_string_list(session->personal,pwl_suggs);

	if (out_n_suggs)
		*out_n_suggs = n_suggs;

	return suggs;
}

/**
 * enchant_dict_add_to_pwl
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your personal dictionary, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_pwl (EnchantDict * dict, const char *const word,
			 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);

	g_return_if_fail (len);
	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);
	enchant_session_add_personal (session, word, len);
	
	if (dict->add_to_personal)
		(*dict->add_to_personal) (dict, word, len);
}

/**
 * enchant_dict_add_to_personal
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your personal dictionary, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 * DEPRECATED. Please use enchant_dict_add_to_pwl() instead.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_personal (EnchantDict * dict, const char *const word,
				  ssize_t len)
{
	enchant_dict_add_to_pwl (dict, word, len);
}

/**
 * enchant_dict_add_to_session
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to this spell-checking session, in UTF-8 encoding
 * @len: The byte length of @word, or -1 for strlen (@word)
 *
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_session (EnchantDict * dict, const char *const word,
				 ssize_t len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (word);

	if (len < 0)
		len = strlen (word);
	
	g_return_if_fail (len);
	
	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);

	enchant_session_add (session, word, len);
	if (dict->add_to_session)
		(*dict->add_to_session) (dict, word, len);
}

/**
 * enchant_dict_is_in_session
 * @dict: A non-null #EnchantDict
 * @word: The word you wish to see if it's in your session
 * @len: the byte length of @word, or -1 for strlen (@word)
 */
ENCHANT_MODULE_EXPORT (int)
enchant_dict_is_in_session (EnchantDict * dict, const char *const word,
				ssize_t len)
{
	EnchantSession * session;

	g_return_val_if_fail (dict, 0);
	g_return_val_if_fail (word, 0);

	if (len < 0)
		len = strlen (word);
	
	g_return_val_if_fail (len, 0);

	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);

	return enchant_session_contains (session, word, len);
}

/**
 * enchant_dict_store_replacement
 * @dict: A non-null #EnchantDict
 * @mis: The non-null word you wish to add a correction for, in UTF-8 encoding
 * @mis_len: The byte length of @mis, or -1 for strlen (@mis)
 * @cor: The non-null correction word, in UTF-8 encoding
 * @cor_len: The byte length of @cor, or -1 for strlen (@cor)
 *
 * Notes that you replaced @mis with @cor, so it's possibly more likely
 * that future occurrences of @mis will be replaced with @cor. So it might
 * bump @cor up in the suggestion list.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_store_replacement (EnchantDict * dict,
				const char *const mis, ssize_t mis_len,
				const char *const cor, ssize_t cor_len)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (mis);
	g_return_if_fail (cor);

	if (mis_len < 0)
		mis_len = strlen (mis);

	if (cor_len < 0)
		cor_len = strlen (cor);

	g_return_if_fail (mis_len);
	g_return_if_fail (cor_len);

	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);
	
	/* if it's not implemented, it's not worth emulating */
	if (dict->store_replacement)
		(*dict->store_replacement) (dict, mis, mis_len, cor, cor_len);
}

/**
 * enchant_dict_free_string_list
 * @dict: A non-null #EnchantDict
 * @string_list: A non-null string list returned from enchant_dict_suggest
 *
 * Releases the string list
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_free_string_list (EnchantDict * dict, char **string_list)
{
	EnchantSession * session;

	g_return_if_fail (dict);
	g_return_if_fail (string_list);
	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);
	g_strfreev(string_list);
}

/**
 * enchant_dict_free_suggestions
 * @dict: A non-null #EnchantDict
 * @suggestions: The non-null suggestion list returned by
 *               'enchant_dict_suggest'
 *
 * Releases the suggestions
 * This function is DEPRECATED. Please use enchant_dict_free_string_list() instead.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_free_suggestions (EnchantDict * dict, char **suggestions)
{
	enchant_dict_free_string_list (dict, suggestions);
}

/**
 * enchant_dict_describe
 * @broker: A non-null #EnchantDict
 * @dict: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Describes an individual dictionary
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_describe (EnchantDict * dict,
			   EnchantDictDescribeFn fn,
			   void * user_data)
{
	EnchantSession * session;
	EnchantProvider * provider;
	GModule *module;

	const char * tag, * name, * desc, * file;

	g_return_if_fail (dict);
	g_return_if_fail (fn);

	session = (EnchantSession*)dict->enchant_private_data;
	enchant_session_clear_error (session);
	provider = session->provider;

	if (provider) 
		{
			module = (GModule *) provider->enchant_private_data;
			file = g_module_name (module);	
			name = (*provider->identify) (provider);
			desc = (*provider->describe) (provider);
		} 
	else 
		{
			file = session->personal_filename;
			name = "Personal Wordlist";
			desc = "Personal Wordlist";
		}
	
	tag = session->language_tag;
	(*fn) (tag, name, desc, file, user_data);
}

/***********************************************************************************/
/***********************************************************************************/

static void
enchant_broker_clear_error (EnchantBroker * broker)
{
	if (broker->error) 
		{
			g_free (broker->error);
			broker->error = NULL;
		}
}

static int
enchant_provider_is_valid(EnchantProvider * provider)
{
	if(provider == NULL)
		{
			g_warning ("EnchantProvider cannot be NULL\n");
			return 0;
		}

	if(provider->identify == NULL)
		{
			g_warning ("EnchantProvider's identify method cannot be NULL\n");
			return 0;
		}

	if(provider->describe == NULL)
		{
			g_warning ("EnchantProvider's describe method cannot be NULL\n");
			return 0;
		}

	return 1;
}

static void
enchant_load_providers_in_dir (EnchantBroker * broker, const char *dir_name)
{
	GModule *module = NULL;
	GDir *dir;
	G_CONST_RETURN char *dir_entry;
	size_t entry_len, g_module_suffix_len;
	
	char * filename;
	
	EnchantProvider *provider;
	EnchantProviderInitFunc init_func;
	EnchantPreConfigureFunc conf_func;
	
	dir = g_dir_open (dir_name, 0, NULL);
	if (!dir) 
		return;
	
	g_module_suffix_len = strlen (G_MODULE_SUFFIX);

	while ((dir_entry = g_dir_read_name (dir)) != NULL)
		{
			provider = 0;

			entry_len = strlen (dir_entry);
			if ((entry_len > g_module_suffix_len) && 
				!strcmp(dir_entry+(entry_len-g_module_suffix_len), G_MODULE_SUFFIX))
				{
					filename = g_build_filename (dir_name, dir_entry, NULL);
					
					module = g_module_open (filename, (GModuleFlags) 0);
					if (module) 
						{
							if (g_module_symbol
								(module, "init_enchant_provider", (gpointer *) (&init_func))
								&& init_func)
								{
									provider = init_func ();
									if (!enchant_provider_is_valid(provider))
										{
											g_warning ("Error loading plugin: %s's init_enchant_provider returned invalid provider.\n", dir_entry);
											if(provider)
												{
													if(provider->dispose)
														provider->dispose(provider);

													provider = NULL;
												}
											g_module_close (module);
										}
								}
							else
								{
									g_module_close (module);
								}
						} 
					else 
						{
							g_warning ("Error loading plugin: %s\n", g_module_error());
						}
					
					g_free (filename);
				}
			if (provider)
				{
					/* optional entry point to allow modules to look for associated files
					 */
					if (g_module_symbol
						(module, "configure_enchant_provider", (gpointer *) (&conf_func))
						&& conf_func)
						{
							conf_func (provider, dir_name);
							if (!enchant_provider_is_valid(provider))
								{
									g_warning ("Error loading plugin: %s's configure_enchant_provider modified provider and it is now invalid.\n", dir_entry);
									if(provider->dispose)
										provider->dispose(provider);

									provider = NULL;
									g_module_close (module);
								}
						}
				}
			if (provider)
				{
					provider->enchant_private_data = (void *) module;
					provider->owner = broker;
					broker->provider_list = g_slist_append (broker->provider_list, (gpointer)provider);
				}
		}
	
	g_dir_close (dir);
}

static void
enchant_load_providers (EnchantBroker * broker)
{
	gchar *user_dir, *home_dir, *system_dir;
	
	/* load USER providers first. since the GSList is ordered,
	   this intentionally gives preference to USER providers */

	home_dir = enchant_get_user_home_dir ();

	if (home_dir) 
		{
			user_dir = g_build_filename (home_dir, ENCHANT_USER_PATH_EXTENSION, NULL);
			enchant_load_providers_in_dir (broker, user_dir);
			g_free (user_dir);
			g_free (home_dir);
		}

	system_dir = enchant_get_module_dir ();
	if (system_dir)
		{
			enchant_load_providers_in_dir (broker, system_dir);
			g_free (system_dir);
		}
}

static void
enchant_load_ordering_from_file (EnchantBroker * broker, const char * file)
{
	char line [1024];
	char * tag, * ordering;

	size_t i, len;

	FILE * f;

	f = g_fopen (file, "r");
	if (!f)
		return;

	while (NULL != fgets (line, sizeof(line), f)) {
		for (i = 0, len = strlen(line); i < len && line[i] != ':'; i++) 
			;

		if (i < len) 
			{
				tag = g_strndup (line, i);
				ordering = g_strndup (line+(i+1), len - i);			
				
				enchant_broker_set_ordering (broker, tag, ordering);
				
				g_free (tag);
				g_free (ordering);
			}
	}
	
	fclose (f);
}

static void
enchant_load_provider_ordering (EnchantBroker * broker)
{
	char * ordering_file, * home_dir, * global_ordering;

	broker->provider_ordering = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

	global_ordering = enchant_get_conf_dir ();
	if (global_ordering) 
		{
			ordering_file = g_build_filename (global_ordering, "enchant.ordering", NULL);
			enchant_load_ordering_from_file (broker, ordering_file);
			g_free (ordering_file);
			g_free (global_ordering);
		}
	
	home_dir = enchant_get_user_home_dir ();
	
	if (home_dir) 
		{
			ordering_file = g_build_filename (home_dir, ENCHANT_USER_PATH_EXTENSION, "enchant.ordering", NULL);
			enchant_load_ordering_from_file (broker, ordering_file);
			g_free (ordering_file);
			g_free (home_dir);
		}
}

static GSList *
enchant_get_ordered_providers (EnchantBroker * broker,
				   const char * const tag)
{
	EnchantProvider *provider;
	GSList * list = NULL, * iter = NULL;

	char * ordering = NULL, ** tokens, *token;
	size_t i;

	ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)tag);
	if (!ordering)
		ordering = (char *)g_hash_table_lookup (broker->provider_ordering, (gpointer)"*");

	if (!ordering) 
		{
			/* return an unordered copy of the list */
			for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
					list = g_slist_append (list, iter->data);
			return list;
		}
	
	tokens = g_strsplit (ordering, ",", 0);
	if (tokens) 
		{
			for (i = 0; tokens[i]; i++) 
				{
					token = g_strstrip(tokens[i]);
					
					for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter)) 
						{
							provider = (EnchantProvider*)iter->data;
							
							if (provider && !strcmp (token, (*provider->identify)(provider)))
								list = g_slist_append (list, (gpointer)provider);
						}
				}
			
			g_strfreev (tokens);
		}
	
	/* providers not in the list need to be appended at the end */
	for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter)) 
		{
			if (!g_slist_find (list, iter->data))
				list = g_slist_append (list, iter->data);
		}

	return list;
}

static void
enchant_dict_destroyed (gpointer data)
{
	EnchantDict *dict;
	EnchantProvider *owner;
	EnchantSession *session;
	
	g_return_if_fail (data);
	
	dict = (EnchantDict *) data;
	session = (EnchantSession*)dict->enchant_private_data;
	owner = session->provider;
	
	if (owner && owner->dispose_dict) 
		(*owner->dispose_dict) (owner, dict);
	else if(session->is_pwl)
		g_free (dict);

	enchant_session_destroy (session);
}

static void
enchant_provider_free (gpointer data, gpointer user_data)
{
	EnchantProvider *provider;
	GModule *module;
	
	g_return_if_fail (data);
	
	provider = (EnchantProvider *) data;	
	module = (GModule *) provider->enchant_private_data;
	
	if (provider->dispose) 
		(*provider->dispose) (provider);
	
	/* close module only after invoking dispose */
	g_module_close (module);
}

/**
 * enchant_broker_init
 *
 * Returns: A new broker object capable of requesting
 * dictionaries from
 */
ENCHANT_MODULE_EXPORT (EnchantBroker *) 
enchant_broker_init (void)
{
	EnchantBroker *broker = NULL;
	
	g_return_val_if_fail (g_module_supported (), NULL);

#ifdef ENABLE_BINRELOC
	{
		static gboolean binreloc_initialized = FALSE;

		if (!binreloc_initialized)
			{
				(void)gbr_init_lib (NULL);
				binreloc_initialized = TRUE;
			}
	}
#endif
	
	broker = g_new0 (EnchantBroker, 1);
	
	broker->dict_map = g_hash_table_new_full (g_str_hash, g_str_equal,
						  g_free, enchant_dict_destroyed);
	
	enchant_load_providers (broker);	
	enchant_load_provider_ordering (broker);

	return broker;
}

/**
 * enchant_broker_free
 * @broker: A non-null #EnchantBroker
 *
 * Destroys the broker object. Must only be called once per broker init
 */
ENCHANT_MODULE_EXPORT (void) 
enchant_broker_free (EnchantBroker * broker)
{
	guint n_remaining;

	g_return_if_fail (broker);       

	n_remaining = g_hash_table_size (broker->dict_map);
	if (n_remaining) 
		{
			g_warning ("%u dictionaries weren't free'd.\n", n_remaining);
		}

	/* will destroy any remaining dictionaries for us */
	g_hash_table_destroy (broker->dict_map);
	g_hash_table_destroy (broker->provider_ordering);
	
	g_slist_foreach (broker->provider_list, enchant_provider_free, NULL);
	g_slist_free (broker->provider_list);

	enchant_broker_clear_error (broker);

	g_free (broker);
}

/**
 * enchant_broker_request_pwl_dict
 *
 * PWL is a personal wordlist file, 1 entry per line
 *
 * @pwl: A non-null pathname in the GLib file name encoding (UTF-8 on Windows)
 *       to the personal wordlist file
 *
 * Returns: 
 */
ENCHANT_MODULE_EXPORT (EnchantDict *)
enchant_broker_request_pwl_dict (EnchantBroker * broker, const char *const pwl)
{
	EnchantSession *session;
	EnchantDict *dict = NULL;

	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (pwl && strlen(pwl), NULL);

	enchant_broker_clear_error (broker);

	dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) pwl);
	if (dict) {
		return dict;
	}

	session = enchant_session_new_with_pwl (NULL, pwl, "Personal Wordlist", TRUE);
	if (!session) 
		{
			broker->error = g_strdup_printf ("Couldn't open personal wordlist '%s'", pwl);
			return NULL;
		}

	session->is_pwl = 1;

	dict = g_new0 (EnchantDict, 1);
	dict->enchant_private_data = (void *)session;

	g_hash_table_insert (broker->dict_map, (gpointer)g_strdup (pwl), dict);

	return dict;
}

static EnchantDict *
_enchant_broker_request_dict (EnchantBroker * broker, const char *const tag)
{
	EnchantDict * dict;
	GSList * list;

	dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) tag);
	if (dict) {
		return dict;
	}

	for (list = enchant_get_ordered_providers (broker, tag); list != NULL; list = g_slist_next (list))
		{
			EnchantProvider * provider;

			provider = (EnchantProvider *) list->data;
			
			if (provider->request_dict)
				{
					dict = (*provider->request_dict) (provider, tag);
					
					if (dict)
						{
							EnchantSession *session;
	
							session = enchant_session_new (provider, tag);
							dict->enchant_private_data = (void*)session;
							g_hash_table_insert (broker->dict_map, (gpointer)g_strdup (tag), dict);
							break;
						}
				}
		}

	g_slist_free (list);

	return dict;
}

/**
 * enchant_broker_request_dict
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be found.
 */
ENCHANT_MODULE_EXPORT (EnchantDict *)
enchant_broker_request_dict (EnchantBroker * broker, const char *const tag)
{
	EnchantDict *dict = NULL;
	char * normalized_tag;

	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (tag && strlen(tag), NULL);

	enchant_broker_clear_error (broker);
	
	normalized_tag = enchant_normalize_dictionary_tag (tag);
	if ((dict = _enchant_broker_request_dict (broker, normalized_tag)) == NULL)
		{
			char * iso_639_only_tag;

			iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);

			dict = _enchant_broker_request_dict (broker, iso_639_only_tag);

			g_free (iso_639_only_tag);
		}

	g_free (normalized_tag);
	
	return dict;
}

/**
 * enchant_broker_describe
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantBrokerDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the Enchant providers and tells
 * you some rudimentary information about them.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_describe (EnchantBroker * broker,
			 EnchantBrokerDescribeFn fn,
			 void * user_data)
{
	GSList *list;
	EnchantProvider *provider;
	GModule *module;

	const char * name, * desc, * file;

	g_return_if_fail (broker);
	g_return_if_fail (fn);

	enchant_broker_clear_error (broker);

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			provider = (EnchantProvider *) list->data;
			module = (GModule *) provider->enchant_private_data;
			
			name = (*provider->identify) (provider);
			desc = (*provider->describe) (provider);
			file = g_module_name (module);
			
			(*fn) (name, desc, file, user_data);
		}
}

/**
 * enchant_broker_list_dicts
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the dictionaries available from
 * all Enchant providers.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_list_dicts (EnchantBroker * broker,
			   EnchantDictDescribeFn fn,
			   void * user_data)
{
	GSList *list;
	GHashTable *tags;
	
	g_return_if_fail (broker);
	g_return_if_fail (fn);

	tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	enchant_broker_clear_error (broker);

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider *provider;
			GModule *module;

			provider = (EnchantProvider *) list->data;
			module = (GModule *) provider->enchant_private_data;

			if (provider->list_dicts)
				{
					const char * tag, * name, * desc, * file;
					size_t n_dicts, i;
					char ** dicts;				       

					dicts = (*provider->list_dicts) (provider, &n_dicts);
					name = (*provider->identify) (provider);
					desc = (*provider->describe) (provider);
					file = g_module_name (module);

					for (i = 0; i < n_dicts; i++)
						{
							tag = dicts[i];
							if (!g_hash_table_lookup (tags, tag))
								{
									g_hash_table_insert (tags, g_strdup (tag), GINT_TO_POINTER(TRUE));
									(*fn) (tag, name, desc, file, user_data);
								}
						}

					enchant_provider_free_string_list (provider, dicts);
				}	
		}

	g_hash_table_destroy (tags);
}

/**
 * enchant_broker_free_dict
 * @broker: A non-null #EnchantBroker
 * @dict: A non-null #EnchantDict
 *
 * Releases the dictionary when you are done using it. Must only be called once per dictionary request
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_free_dict (EnchantBroker * broker, EnchantDict * dict)
{
	EnchantSession * session;

	g_return_if_fail (broker);
	g_return_if_fail (dict);

	enchant_broker_clear_error (broker);

	session = (EnchantSession*)dict->enchant_private_data;
	
	if (session->provider)
		g_hash_table_remove (broker->dict_map, session->language_tag);
	else
		g_hash_table_remove (broker->dict_map, session->personal_filename);
}

static int
_enchant_provider_dictionary_exists (EnchantProvider * provider,
					 const char * const tag)
{
	int exists = 0;

	if (provider->dictionary_exists)
		{
			exists = (*provider->dictionary_exists) (provider, tag);
		}
	else if (provider->list_dicts)
		{
			size_t n_dicts, i;
			char ** dicts;				       
			
			dicts = (*provider->list_dicts) (provider, &n_dicts);
			
			for (i = 0; (i < n_dicts) && !exists; i++)
				{
					if (!strcmp(dicts[i], tag)) 
						exists = 1;
				}
			
			enchant_provider_free_string_list (provider, dicts);
		}
	else if (provider->request_dict)
		{
			EnchantDict *dict;

			dict = (*provider->request_dict) (provider, tag);
			if (dict)
				{
					if (provider->dispose_dict) 
						(*provider->dispose_dict) (provider, dict);
					exists = 1;
				}
		}

	return exists;
}

static int
_enchant_broker_dict_exists (EnchantBroker * broker,
				 const char * const tag)
{
	GSList * list;

	/* don't query the providers if it is an empty string */
	if (tag == NULL || *tag == '\0') {
		return 0;
	}

	/* don't query the providers if we can just do a quick map lookup */
	if (g_hash_table_lookup (broker->dict_map, (gpointer) tag) != NULL) {
		return 1;
	}

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			EnchantProvider * provider;

			provider = (EnchantProvider *) list->data;

			if (_enchant_provider_dictionary_exists (provider, tag))
				{
					return 1;
				}
		}

	return 0;
}

/**
 * enchant_broker_dict_exists
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 *
 * Return existance of the requested dictionary (1 == true, 0 == false)
 */
ENCHANT_MODULE_EXPORT (int)
enchant_broker_dict_exists (EnchantBroker * broker,
				const char * const tag)
{
	char * normalized_tag;
	int exists = 0;

	g_return_val_if_fail (broker, 0);
	g_return_val_if_fail (tag && strlen(tag), 0);

	enchant_broker_clear_error (broker);

	normalized_tag = enchant_normalize_dictionary_tag (tag);

	if ((exists = _enchant_broker_dict_exists (broker, normalized_tag)) == 0)
		{
			char * iso_639_only_tag;

			iso_639_only_tag = enchant_iso_639_from_tag (normalized_tag);

			if (strcmp (normalized_tag, iso_639_only_tag) != 0)
				{
					exists = _enchant_broker_dict_exists (broker, iso_639_only_tag);
				}

			g_free (iso_639_only_tag);
		}

	g_free (normalized_tag);
	return exists;
}

/**
 * enchant_broker_set_ordering
 * @broker: A non-null #EnchantBroker
 * @tag: A non-null language tag (en_US)
 * @ordering: A non-null ordering (aspell,myspell,ispell,uspell,hspell)
 *
 * Declares a preference of dictionaries to use for the language
 * described/referred to by @tag. The ordering is a comma delimited
 * list of provider names. As a special exception, the "*" tag can
 * be used as a language tag to declare a default ordering for any
 * language that does not explictly declare an ordering.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_set_ordering (EnchantBroker * broker,
				 const char * const tag,
				 const char * const ordering)
{
	char * tag_dupl;
	char * ordering_dupl;

	g_return_if_fail (broker);
	g_return_if_fail (tag && strlen(tag));
	g_return_if_fail (ordering && strlen(ordering));

	enchant_broker_clear_error (broker);

	tag_dupl = enchant_normalize_dictionary_tag (tag);

	ordering_dupl = g_strdup (ordering);
	ordering_dupl = g_strstrip (ordering_dupl);

	if (tag_dupl && strlen(tag_dupl) &&
		ordering_dupl && strlen(ordering_dupl)) 
		{			
			/* we will free ordering_dupl && tag_dupl when the hash is destroyed */
			g_hash_table_insert (broker->provider_ordering, (gpointer)tag_dupl,
						 (gpointer)(ordering_dupl));		       
		} 
	else 
		{
			g_free (tag_dupl);
			g_free (ordering_dupl);
		}
}

/**
 * enchant_provider_set_error
 * @provider: A non-null provider
 * @err: A non-null error message
 * 
 * Sets the current runtime error to @err. This API is private to
 * the providers.
 */
ENCHANT_MODULE_EXPORT(void)
enchant_provider_set_error (EnchantProvider * provider, const char * const err)
{
	EnchantBroker * broker;

	g_return_if_fail (provider);
	g_return_if_fail (err);

	broker = provider->owner;
	g_return_if_fail (broker);
	
	enchant_broker_clear_error (broker);
	broker->error = g_strdup (err);	
}

/**
 * enchant_broker_get_error
 * @broker: A non-null broker
 *
 * Returns a const char string or NULL describing the last exception.
 * WARNING: error is transient and is likely cleared as soon as the 
 * next broker operation happens
 */
ENCHANT_MODULE_EXPORT(char *)
enchant_broker_get_error (EnchantBroker * broker)
{
	g_return_val_if_fail (broker, NULL);
	
	return broker->error;
}

/* private. returned string should be free'd with g_free */
ENCHANT_MODULE_EXPORT(char *)
enchant_get_user_language(void)
{
	char * locale = NULL;
	
#if defined(G_OS_WIN32)
	if(!locale)
		locale = g_win32_getlocale ();
#endif       
	
	if(!locale)
		locale = g_strdup (g_getenv ("LANG"));
	
#if defined(HAVE_LC_MESSAGES)
	if(!locale)
		locale = g_strdup (setlocale (LC_MESSAGES, NULL));
#endif

	if(!locale)
		locale = g_strdup (setlocale (LC_ALL, NULL));

	if(!locale || strcmp(locale, "C") == 0) {
		g_free(locale);
		locale = g_strdup("en");
	}
		
	return locale;
}


/**
 * enchant_get_prefix_dir
 *
 * Returns a string giving the location of the base directory
 * of the enchant installation.  This corresponds roughly to 
 * the --prefix option given to ./configure when enchant is
 * compiled, except it is determined at runtime based on the location
 * of the enchant library.
 *
 * Returns: the prefix dir if it can be determined, or %null otherwise. Must be free'd.
 *
 * This API is private to the providers.
 *
 */
ENCHANT_MODULE_EXPORT (char *)
enchant_get_prefix_dir(void)
{
	char * prefix = NULL;

#ifdef _WIN32
	/* Dynamically locate library and return containing directory */
	HINSTANCE hInstance = GetModuleHandle(L"libenchant");
	if(hInstance != NULL)
	{
		WCHAR dll_path[MAX_PATH];
	  
		if(GetModuleFileName(hInstance,dll_path,MAX_PATH))
		{
			gchar* utf8_dll_path = g_utf16_to_utf8 (dll_path, -1, NULL, NULL, NULL);
			prefix = g_path_get_dirname(utf8_dll_path);
			g_free(utf8_dll_path);
		}
	}
#elif defined(ENABLE_BINRELOC)
	/* Use standard binreloc PREFIX macro */
	prefix = gbr_find_prefix(NULL);
#endif

	return prefix;
}
