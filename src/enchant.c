/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Dom Lachowicz
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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
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

#include "enchant.h"
#include "enchant-provider.h"

/* TODO: use the win32 registry in these following functions PLUS the
 * macro, if it exists (registry gets precedence)
 */

/**
 * enchant_get_user_home_dir
 *
 * Returns the user's home directory, or %null. Returned value
 * must be free'd
 */
ENCHANT_MODULE_EXPORT (char *)
enchant_get_user_home_dir (void)
{
	const char * home_dir = g_get_home_dir ();

	if (!home_dir)
		return NULL;
	return g_strdup (home_dir);
}

static char *
enchant_get_module_dir (void)
{
	return g_strdup (ENCHANT_GLOBAL_MODULE_DIR);
}

static char *
enchant_get_conf_dir (void)
{
	return g_strdup (ENCHANT_GLOBAL_ORDERING);
}

/**
 * enchant_dict_check
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to correct, in UTF-8 encoding
 * @len: The non-zero byte length of @word
 *
 * Will return an "incorrect" value if any of those pre-conditions
 * are not met.
 *
 * Returns: 0 if the word is correctly spelled, non-zero if not
 */
ENCHANT_MODULE_EXPORT (int)
enchant_dict_check (EnchantDict * dict, const char *const word, size_t len)
{
	g_return_val_if_fail (dict, 1);
	g_return_val_if_fail (word, 1);
	g_return_val_if_fail (len, 1);
	
	if (dict->check)
		{
			return (*dict->check) (dict, word, len);
		}
	
	return 1;
}

/**
 * enchant_dict_suggest
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to find suggestions for, in UTF-8 encoding
 * @len: The non-zero byte length of @word
 * @out_n_suggs: The non-null location to store the # of suggestions returned
 *
 * Will return an %null value if any of those pre-conditions
 * are not met.
 *
 * Returns: A %null terminated list of UTF-8 encoded suggestions, or %null
 */
ENCHANT_MODULE_EXPORT (char **)
enchant_dict_suggest (EnchantDict * dict, const char *const word,
		      size_t len, size_t * out_n_suggs)
{
	g_return_val_if_fail (dict, NULL);
	g_return_val_if_fail (word, NULL);
	g_return_val_if_fail (len, NULL);
	g_return_val_if_fail (out_n_suggs, NULL);
	
	if (dict->suggest)
		{
			return (*dict->suggest) (dict, word, len, out_n_suggs);
		}
	
	*out_n_suggs = 0;
	return NULL;
}

/**
 * enchant_dict_add_to_personal
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your personal dictionary, in UTF-8 encoding
 * @len: The non-zero byte length of @word
 *
 * An implementation of "add_to_personal" is not guaranteed to work in all cases.
 * This function's implementation may vary by provider.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_personal (EnchantDict * dict, const char *const word,
			      size_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);
	g_return_if_fail (len);
	
	if (dict->add_to_personal)
		{
			(*dict->add_to_personal) (dict, word, len);
		}
}

/**
 * enchant_dict_add_to_session
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to this spell-checking session, in UTF-8 encoding
 * @len: The non-zero byte length of @word
 *
 * An implementation of "add_to_session" is not guaranteed to work in all cases.
 * This function's implementation may vary by provider.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_add_to_session (EnchantDict * dict, const char *const word,
			     size_t len)
{
	g_return_if_fail (dict);
	g_return_if_fail (word);
	g_return_if_fail (len);
	
	if (dict->add_to_session)
		{
			(*dict->add_to_session) (dict, word, len);
		}
}

/**
 * enchant_dict_add_to_session
 * @dict: A non-null #EnchantDict
 * @mis: The non-null word you wish to add a correction for, in UTF-8 encoding
 * @mis_len: The non-zero byte length of @mis
 * @cor: The non-null correction word, in UTF-8 encoding
 * @cor_len: The non-zero byte length of @cor
 *
 * An implementation of "store_replacement" is not guaranteed to work in all cases.
 * This function's implementation may vary by provider.
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_store_replacement (EnchantDict * dict,
				const char *const mis, size_t mis_len,
				const char *const cor, size_t cor_len)
{
	g_return_if_fail (dict);
	g_return_if_fail (mis);
	g_return_if_fail (mis_len);
	g_return_if_fail (cor);
	g_return_if_fail (cor_len);
	
	if (dict->store_replacement)
		{
			(*dict->store_replacement) (dict, mis, mis_len, cor, cor_len);
		}
}

/**
 * enchant_dict_free_suggestions
 * @dict: A non-null #EnchantDict
 * @suggestions: The non-null suggestion list returned by
 *               'enchant_dict_suggest'
 *
 * Releases the suggestions
 */
ENCHANT_MODULE_EXPORT (void)
enchant_dict_free_suggestions (EnchantDict * dict, char **suggestions)
{
	g_return_if_fail (dict);
	g_return_if_fail (suggestions);
	
	if (dict->free_suggestions)
		{
			(*dict->free_suggestions) (dict, suggestions);
		}
}

/***********************************************************************************/
/***********************************************************************************/

struct str_enchant_broker
{
	GSList *provider_list;	/* list of all of the spelling backend providers */
	GHashTable *dict_map;		/* map of language tag -> dictionary */
	GHashTable *provider_ordering; /* map of language tag -> provider order */
};

typedef EnchantProvider *(*EnchantProviderInitFunc) ();

static void
enchant_load_providers_in_dir (EnchantBroker * broker, const char *dir_name)
{
	GModule *module;
	GDir *dir;
	G_CONST_RETURN char *dir_entry;
	size_t entry_len, g_module_suffix_len;
	
	char * filename;
	
	EnchantProvider *provider;
	EnchantProviderInitFunc init_func;
	
	dir = g_dir_open (dir_name, 0, NULL);
	if (!dir) 
		{
			return;
		}
	
	g_module_suffix_len = strlen (G_MODULE_SUFFIX);

	while ((dir_entry = g_dir_read_name (dir)) != NULL)
		{
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
									if (provider)
										{
											provider->enchant_private_data = (void *) module;
											broker->provider_list = g_slist_append (broker->provider_list, (gpointer)provider);
										}
								}
							else
								{
									g_module_close (module);
								}
						} 
					else 
						{
							g_warning ("Module error: %s\n", g_module_error());
						}
					
					g_free (filename);
				}
		}
	
	g_dir_close (dir);
}

static void
enchant_load_providers (EnchantBroker * broker)
{
	gchar *user_dir, *home_dir;       
	
	/* load USER providers first. since the GSList is ordered,
	   this intentionally gives preference to USER providers */

	home_dir = enchant_get_user_home_dir ();

	if (home_dir) {
		user_dir = g_build_filename (home_dir, ".enchant", NULL);
		enchant_load_providers_in_dir (broker, user_dir);
		g_free (user_dir);
		g_free (home_dir);
	}

	enchant_load_providers_in_dir (broker, ENCHANT_GLOBAL_MODULE_DIR);
}

static void
enchant_provider_order_destroyed (gpointer data)
{
	g_free (data);
}

static void
enchant_load_ordering_from_file (EnchantBroker * broker, const char * file)
{
	char line [1024];
	char * tag, * ordering;

	size_t i, len;

	FILE * f;

	f = fopen (file, "r");
	if (!f)
		return;

	while (NULL != fgets (line, sizeof(line), f)) {
		for (i = 0, len = strlen(line); i < len && line[i] != ':'; i++) {
			;
		}

		if (i < len) {
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
	char * ordering_file, * home_dir;

	broker->provider_ordering = g_hash_table_new_full (g_str_hash, g_str_equal,
							   enchant_provider_order_destroyed, enchant_provider_order_destroyed);

	ordering_file = g_build_filename (ENCHANT_GLOBAL_ORDERING, "enchant.ordering", NULL);
	enchant_load_ordering_from_file (broker, ordering_file);
	g_free (ordering_file);

	home_dir = enchant_get_user_home_dir ();

	if (home_dir) {
		ordering_file = g_build_filename (home_dir, ".enchant", "enchant.ordering", NULL);
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

	if (!ordering) {
		/* return an unordered copy of the list */
		for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter))
			{
				list = g_slist_append (list, iter->data);
			}
		return list;
	}

	tokens = g_strsplit (ordering, ",", 0);
	if (tokens) {
		for (i = 0; tokens[i]; i++) {
			token = tokens[i];

			for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter)) {
				provider = (EnchantProvider*)iter->data;
 
				if (provider && !strcmp (token, (*provider->identify)(provider))) {
					list = g_slist_append (list, (gpointer)provider);
				}
			}
		}

		g_strfreev (tokens);
	}

	/* providers not in the list need to be appended at the end */
	for (iter = broker->provider_list; iter != NULL; iter = g_slist_next (iter)) {
		if (!g_slist_find (list, iter->data))
			list = g_slist_append (list, iter->data);
	}

	return list;
}

static void
enchant_dict_key_destroyed (gpointer data)
{
	g_free (data);
}

static void
enchant_dict_destroyed (gpointer data)
{
	EnchantDict *dict;
	EnchantProvider *owner;
	
	g_return_if_fail (data);
	
	dict = (EnchantDict *) data;
	owner = dict->owner;
	
	if (owner->dispose_dict) 
		{
			(*owner->dispose_dict) (owner, dict);
		}
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
		{
			(*provider->dispose) (provider);
		}
	
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
	
	broker = g_new0 (EnchantBroker, 1);
	
	broker->dict_map = g_hash_table_new_full (g_str_hash, g_str_equal,
						  enchant_dict_key_destroyed, enchant_dict_destroyed);
	
	enchant_load_providers (broker);
	
	enchant_load_provider_ordering (broker);

	return broker;
}

/**
 * enchant_broker_term
 * @broker: A non-null #EnchantBroker
 *
 * Destroys the broker object
 */
ENCHANT_MODULE_EXPORT (void) 
enchant_broker_term (EnchantBroker * broker)
{
	g_return_if_fail (broker);
	
	/* will destroy the dictionaries for us */
	g_hash_table_destroy (broker->dict_map);
	g_hash_table_destroy (broker->provider_ordering);
	
	g_slist_foreach (broker->provider_list, enchant_provider_free, NULL);
	g_slist_free (broker->provider_list);
	
	g_free (broker);
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
	EnchantProvider *provider;
	EnchantDict *dict = NULL;
	GSList *list = NULL;
	
	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (tag && strlen(tag), NULL);
	
	dict = (EnchantDict*)g_hash_table_lookup (broker->dict_map, (gpointer) tag);
	if (dict)
		{
			return dict;
		}
	
	for (list = enchant_get_ordered_providers (broker, tag); list != NULL; list = g_slist_next (list))
		{
			provider = (EnchantProvider *) list->data;
			
			if (provider->request_dict)
				{
					dict = (*provider->request_dict) (provider, tag);
					
					if (dict)
						{
							dict->owner = provider;
							g_hash_table_insert (broker->dict_map, (gpointer)g_strdup (tag), dict);
							break;
						}
				}
		}

	g_slist_free (list);
	
	/* Nothing found */
	return dict;
}

/**
 * enchant_broker_describe
 * @broker: A non-null #EnchantBroker
 * @dict: A non-null #EnchantBrokerDescribeFn
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
 * enchant_broker_release_dict
 * @broker: A non-null #EnchantBroker
 * @dict: A non-null #EnchantDict
 *
 * Releases the dictionary when you are done using it
 */
ENCHANT_MODULE_EXPORT (void)
enchant_broker_release_dict (EnchantBroker * broker, EnchantDict * dict)
{
	g_return_if_fail (broker);
	g_return_if_fail (dict);
	
	/* this will be a noop for now, perhaps indefinitely */
}

/**
 * enchant_broker_dictionary_exists
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for ("en_US", "de_DE", ...)
 *
 * Return existance of the requested dictionary
 */
ENCHANT_MODULE_EXPORT (EnchantDictStatus)
enchant_broker_dictionary_status (EnchantBroker * broker,
				  const char * const tag)
{
	/* start off pessimistic */
	EnchantDictStatus best_status = EDS_DOESNT_EXIST, status = EDS_DOESNT_EXIST;
	EnchantProvider *provider;
	GSList *list;

	g_return_val_if_fail (broker, EDS_UNKNOWN);
	g_return_val_if_fail (tag && strlen(tag), EDS_UNKNOWN);

	/* don't query the providers if we can just do a quick map lookup */
	if (g_hash_table_lookup (broker->dict_map, (gpointer) tag) != NULL)
		return EDS_EXISTS;

	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			provider = (EnchantProvider *) list->data;

			if (provider->dictionary_status)
				{
					status = (*provider->dictionary_status) (provider, tag);
					if (status == EDS_EXISTS)
						return EDS_EXISTS;
					else if (status == EDS_UNKNOWN)
						best_status = EDS_UNKNOWN;
				}
			else
				{
					/* no query routine implemented, return so-so value */
					best_status = EDS_UNKNOWN;
				}
		}


	return best_status;
}

/**
 * enchant_broker_declare_ordering
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

	tag_dupl = g_strdup (tag);
	ordering_dupl = g_strdup (ordering);

	tag_dupl = g_strstrip (tag_dupl);
	ordering_dupl = g_strstrip (ordering_dupl);

	if (tag_dupl && strlen(tag_dupl) &&
	    ordering_dupl && strlen(ordering_dupl)) {

		g_hash_table_insert (broker->provider_ordering, (gpointer)tag_dupl,
				     g_strdup (ordering_dupl));

		/* we will free ordering_dupl && tag_dupl when the hash is destroyed */
	} else {
		g_free (tag_dupl);
		g_free (ordering_dupl);
	}
}
