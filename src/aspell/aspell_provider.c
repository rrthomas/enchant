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
#include <pspell/pspell.h>

#include "enchant.h"
#include "enchant-provider.h"

ENCHANT_PLUGIN_DECLARE("Pspell")

static int
aspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	PspellManager *manager;
	int val;

	manager = (PspellManager *) me->user_data;
	
	val = pspell_manager_check (manager, word, len);
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
aspell_dict_store_replacement (struct str_enchant_dict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
	PspellManager *manager;
	
	manager = (PspellManager *) me->user_data;
	pspell_manager_store_replacement (manager, mis, mis_len,
					  cor, cor_len);
	pspell_manager_save_all_word_lists (manager);
}

static void
aspell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	g_strfreev (str_list);
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
			/*
			  g_warning ("Aspell Enchant backend error when requesting '%s' dictionary: %s\n",
			  tag, pspell_error_message(spell_error));
			*/
			enchant_provider_set_error (me, pspell_error_message(spell_error));
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
	dict->free_suggestions = aspell_dict_free_suggestions;
	
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

static int
aspell_provider_dictionary_status (struct str_enchant_provider * me,
				   const char *const tag)
{
	/* TODO: get kevina to apply my patch */
	EnchantDict * dict;
	int status = 0;

	dict = aspell_provider_request_dict (me, tag);
	if (dict)
		status = 1;

	aspell_provider_dispose_dict (me, dict);

	return status;
}

static void
aspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static char *
aspell_provider_identify (EnchantProvider * me)
{
	return "aspell";
}

static char *
aspell_provider_describe (EnchantProvider * me)
{
	return "Aspell Provider";
}

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = aspell_provider_dispose;
	provider->request_dict = aspell_provider_request_dict;
	provider->dispose_dict = aspell_provider_dispose_dict;
	provider->dictionary_status = aspell_provider_dictionary_status;
	provider->identify = aspell_provider_identify;
	provider->describe = aspell_provider_describe;

	return provider;
}

#ifdef __cplusplus
}
#endif
