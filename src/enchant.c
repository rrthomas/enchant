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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gmodule.h>

#include "enchant.h"

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
};

typedef EnchantProvider *(*EnchantProviderInitFunc) ();

static void
enchant_load_providers_in_dir (EnchantBroker * broker, const char *dir_name)
{
	GModule *module;
	GDir *dir;
	G_CONST_RETURN char *dir_entry;
	
	char * filename;
	
	EnchantProvider *provider;
	EnchantProviderInitFunc init_func;
	
	dir = g_dir_open (dir_name, 0, NULL);
	if (!dir) 
		{
			return;
		}
	
	while ((dir_entry = g_dir_read_name (dir)) != NULL)
		{
			if (/* g_file_test (dir_entry, G_FILE_TEST_EXISTS) &&
			       !g_file_test (dir_entry,G_FILE_TEST_IS_DIR) && */
			    strstr (dir_entry, G_MODULE_SUFFIX) != NULL)
				{
					filename = g_build_filename (dir_name, dir_entry, NULL);
					
					/* this is a module we can try to load */
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
	gchar *user_dir;
	
	enchant_load_providers_in_dir (broker, ENCHANT_GLOBAL_MODULE_DIR);
	
	user_dir = g_strdup_printf ("%s%s.enchant", g_get_home_dir (), 
				    G_DIR_SEPARATOR_S);
	enchant_load_providers_in_dir (broker, user_dir);
	g_free (user_dir);
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
						  NULL, enchant_dict_destroyed);
	
	enchant_load_providers (broker);
	
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
	EnchantDict *dict;
	GSList *list;
	
	g_return_val_if_fail (broker, NULL);
	g_return_val_if_fail (tag, NULL);
	g_return_val_if_fail (strlen (tag), NULL);
	
	dict = g_hash_table_lookup (broker->dict_map, (gpointer) tag);
	if (dict)
		{
			return dict;
		}
	
	for (list = broker->provider_list; list != NULL; list = g_slist_next (list))
		{
			provider = (EnchantProvider *) list->data;
			dict = (*provider->request_dict) (provider, tag);
			
			if (dict)
				{
					dict->owner = provider;
					g_hash_table_insert (broker->dict_map, (gpointer) tag, dict);
					return dict;
				}
		}
	
	/* Nothing found */
	return NULL;
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
