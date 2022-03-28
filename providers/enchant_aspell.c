/* enchant
 * Copyright (C) 2003,2004 Dom Lachowicz
 * Copyright (C) 2017-2024 Reuben Thomas
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders
 * give permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * This is the GNU Aspell Enchant Backend.
 * GNU Aspell is by Kevin Atkinson.  See http://aspell.net/
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <aspell.h>

#include "enchant-provider.h"


EnchantProvider *init_enchant_provider (void);

static int
aspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	AspellSpeller *manager = (AspellSpeller *) me->user_data;

	char *normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	int val = aspell_speller_check (manager, normalizedWord, strlen(normalizedWord));
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
	AspellSpeller *manager = (AspellSpeller *) me->user_data;

	char *normalizedWord = g_utf8_normalize (word, len, G_NORMALIZE_NFC);
	const AspellWordList *word_list = aspell_speller_suggest (manager, normalizedWord, strlen(normalizedWord));
	g_free(normalizedWord);

	char **sugg_arr = NULL;
	if (word_list)
		{
			AspellStringEnumeration *suggestions = aspell_word_list_elements (word_list);
			if (suggestions)
				{
					size_t n_suggestions = aspell_word_list_size (word_list);
					*out_n_suggs = n_suggestions;

					if (n_suggestions)
						{
							sugg_arr = g_new0 (char *, n_suggestions + 1);

							for (size_t i = 0; i < n_suggestions; i++)
								{
									const char *sugg = aspell_string_enumeration_next (suggestions);
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
aspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	AspellSpeller *manager = (AspellSpeller *) me->user_data;
	aspell_speller_add_to_session (manager, word, len);
}

static EnchantDict *
aspell_provider_request_dict (EnchantProvider * me _GL_UNUSED, const char *const tag)
{
	AspellConfig *spell_config = new_aspell_config ();
	aspell_config_replace (spell_config, "master", tag);
	aspell_config_replace (spell_config, "encoding", "utf-8");

	AspellCanHaveError *spell_error = new_aspell_speller (spell_config);
	delete_aspell_config (spell_config);

	if (aspell_error_number (spell_error) != 0)
		{
			enchant_provider_set_error (me, aspell_error_message (spell_error));
			delete_aspell_can_have_error(spell_error);
			return NULL;
		}

	AspellSpeller *manager = to_aspell_speller (spell_error);

	EnchantDict *dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) manager;
	dict->check = aspell_dict_check;
	dict->suggest = aspell_dict_suggest;
	dict->add_to_session = aspell_dict_add_to_session;

	return dict;
}

static void
aspell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED, EnchantDict * dict)
{
	AspellSpeller *manager = (AspellSpeller *) dict->user_data;
	delete_aspell_speller (manager);

	g_free (dict);
}

static char **
aspell_provider_list_dicts (EnchantProvider * me _GL_UNUSED,
			    size_t * out_n_dicts)
{
	AspellConfig * spell_config = new_aspell_config ();
	AspellDictInfoList * dlist = get_aspell_dict_info_list (spell_config);

	*out_n_dicts = 0;
	AspellDictInfoEnumeration * dels = aspell_dict_info_list_elements (dlist);

	/* Note: aspell_dict_info_list_size() is unimplemented: https://github.com/GNUAspell/aspell/issues/155 */
	const AspellDictInfo * entry;
	while ( (entry = aspell_dict_info_enumeration_next(dels)) != 0)
		(*out_n_dicts)++;
	delete_aspell_dict_info_enumeration (dels);

	char ** out_list = NULL;

	if (*out_n_dicts) {
		out_list = g_new0 (char *, *out_n_dicts + 1);
		dels = aspell_dict_info_list_elements (dlist);

		for (size_t i = 0; i < *out_n_dicts; i++) {
			entry = aspell_dict_info_enumeration_next (dels);
			out_list[i] = g_strdup (entry->name);
		}

		delete_aspell_dict_info_enumeration (dels);
	}

	delete_aspell_config (spell_config);

	return out_list;
}

static void
aspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
aspell_provider_identify (EnchantProvider * me _GL_UNUSED)
{
	return "aspell";
}

static const char *
aspell_provider_describe (EnchantProvider * me _GL_UNUSED)
{
	return "Aspell Provider";
}

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = g_new0 (EnchantProvider, 1);
	provider->dispose = aspell_provider_dispose;
	provider->request_dict = aspell_provider_request_dict;
	provider->dispose_dict = aspell_provider_dispose_dict;
	provider->identify = aspell_provider_identify;
	provider->describe = aspell_provider_describe;
	provider->list_dicts = aspell_provider_list_dicts;

	return provider;
}
