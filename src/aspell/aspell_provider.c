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
#include <pspell/pspell.h>
#include "enchant.h"

static int
pspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	
	if (1 == pspell_manager_check (manager, word, len))
		return 0;
	
	return 1;
}

static char **
pspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	PspellManager *manager;
	
	const PspellWordList *word_list;
	PspellStringEmulation *suggestions;
	
	char **sugg_arr = NULL;
	size_t n_suggestions, i;
	const char *sugg;
	
	manager = (PspellManager *) me->user_data;
	
	word_list = pspell_manager_suggest (manager, word, len);
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
pspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_add_to_personal (manager, word, len);
}

static void
pspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_add_to_session (manager, word, len);
}

static void
pspell_dict_store_replacement (struct str_enchant_dict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_store_replacement (manager, mis, mis_len,
					  cor, cor_len);
}

static void
pspell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	g_strfreev (str_list);
}

static EnchantDict *
pspell_provider_request_dict (EnchantProvider * me, const char *const tag)
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
	
	dict = g_new (EnchantDict, 1);
	dict->user_data = (void *) manager;
	dict->check = pspell_dict_check;
	dict->suggest = pspell_dict_suggest;
	dict->add_to_personal = pspell_dict_add_to_personal;
	dict->add_to_session = pspell_dict_add_to_session;
	dict->store_replacement = pspell_dict_store_replacement;
	dict->free_suggestions = pspell_dict_free_suggestions;
	
	return dict;
}

static void
pspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	PspellManager *manager;
	
	manager = (PspellManager *) dict->user_data;
	delete_pspell_manager (manager);
	
	g_free (dict);
}

static void
pspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = pspell_provider_dispose;
	provider->request_dict = pspell_provider_request_dict;
	provider->dispose_dict = pspell_provider_dispose_dict;
	
	return provider;
}
