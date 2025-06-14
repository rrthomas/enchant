/* enchant
 * Copyright (C) 2003,2004 Dom Lachowicz
 *               2006-2007 Harri Pitkänen <hatapitk@iki.fi>
 *               2006 Anssi Hannula <anssi.hannula@gmail.com>
 *               2017 Børre Gaup <borre.gaup@uit.no>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <libvoikko/voikko.h>

#include "enchant-provider.h"

/**
 * Voikko is a spell checker for VFST and HFST format dictionaries. More information is available from:
 *
 * https://voikko.sourceforge.net/
 */

static int
voikko_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	char *word_nul = g_strndup(word, len);
	int result = voikkoSpellCstr((struct VoikkoHandle *)me->user_data, word_nul);
	free(word_nul);
	if (result == VOIKKO_SPELL_FAILED)
		return 1;
	else if (result == VOIKKO_SPELL_OK)
		return 0;
	else
		return -1;
}

static char **
voikko_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	char *word_nul = g_strndup(word, len);
	char **voikko_sugg_arr = voikkoSuggestCstr((struct VoikkoHandle *)me->user_data, word_nul);
	free(word_nul);
	if (voikko_sugg_arr == NULL)
		return NULL;
	for (*out_n_suggs = 0; voikko_sugg_arr[*out_n_suggs] != NULL; (*out_n_suggs)++);

	char **sugg_arr = calloc(sizeof (char *), *out_n_suggs + 1);
	if (sugg_arr == NULL)
		return NULL;
	for (size_t i = 0; i < *out_n_suggs; i++) {
		sugg_arr[i] = strdup (voikko_sugg_arr[i]);
	}
	voikkoFreeCstrArray (voikko_sugg_arr);
	return sugg_arr;
}

static void
voikko_provider_dispose_dict (EnchantProvider * me _GL_UNUSED, EnchantDict * dict)
{
	voikkoTerminate((struct VoikkoHandle *)dict->user_data);
}

static char **
voikko_provider_list_dicts (EnchantProvider * me, size_t * out_n_dicts)
{
	*out_n_dicts = 0;
	char * user_dict_dir = enchant_provider_get_user_dict_dir (me);
	char ** voikko_langs = voikkoListSupportedSpellingLanguages (user_dict_dir);
	g_free (user_dict_dir);

	for (size_t i = 0; voikko_langs[i] != NULL; i++)
		(*out_n_dicts)++;

	char ** out_list = g_new0 (char *, *out_n_dicts + 1);
	for (size_t i = 0; i < *out_n_dicts; i++)
		out_list[i] = strdup (voikko_langs[i]);
	voikkoFreeCstrArray(voikko_langs);

	return out_list;
}

static int
voikko_provider_dictionary_exists (EnchantProvider * me,
				   const char *const tag)
{
	char * user_dict_dir = enchant_provider_get_user_dict_dir (me);
	char ** voikko_langs = voikkoListSupportedSpellingLanguages (user_dict_dir);
	g_free (user_dict_dir);

	int exists = 0;
	for (size_t i = 0; voikko_langs[i] != NULL; i++) {
		if (strncmp (tag, voikko_langs[i], strlen (tag)) == 0) {
			exists = 1;
			break;
		}
	}
	voikkoFreeCstrArray(voikko_langs);

	return exists;
}

static EnchantDict *
voikko_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	const char * voikko_error;

	if (!voikko_provider_dictionary_exists (me, tag)) {
		return NULL;
	}

	char * user_dict_dir = enchant_provider_get_user_dict_dir (me);
	struct VoikkoHandle *voikko_handle = voikkoInit (&voikko_error, tag, user_dict_dir);
	g_free (user_dict_dir);
	if (voikko_handle == NULL) {
		enchant_provider_set_error (me, voikko_error);
		return NULL;
	}

	EnchantDict *dict = enchant_broker_new_dict (me->owner);
	if (dict == NULL)
		return NULL;
	dict->user_data = (void *)voikko_handle;
	dict->check = voikko_dict_check;
	dict->suggest = voikko_dict_suggest;

	return dict;
}

static const char *
voikko_provider_identify (EnchantProvider * me _GL_UNUSED)
{
	return "voikko";
}

static const char *
voikko_provider_describe (EnchantProvider * me _GL_UNUSED)
{
	return "Voikko Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = enchant_provider_new ();
	if (provider == NULL)
		return NULL;
	provider->request_dict = voikko_provider_request_dict;
	provider->dispose_dict = voikko_provider_dispose_dict;
	provider->dictionary_exists = voikko_provider_dictionary_exists;
	provider->identify = voikko_provider_identify;
	provider->describe = voikko_provider_describe;
	provider->list_dicts = voikko_provider_list_dicts;

	return provider;
}
