/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Dom Lachowicz, Raphael Finkel
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

// TODO: Language should have a description file indicating appropriate options
// TODO: don't use BUFLEN or strcat

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include "enchant.h"

// from uspell
#include "utf8convert.h"
#include "uniprops.h"
#include "uspell.h"

static const size_t MAXALTERNATIVE = 20; // we won't return more than this number of suggests

#ifndef BUFLEN
#define BUFLEN 1024
#endif

static int
uspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	class uSpell *manager;
	wide_t bigBuf[BUFLEN], upperBuf[BUFLEN];
	int length;
	
	manager = (class uSpell *) me->user_data;
	length = utf8_wide(bigBuf, reinterpret_cast<const utf8_t *>(word), BUFLEN);
	if (manager->isSpelledRight(bigBuf, length)) 
		return 0; // correct the first time
	toUpper(upperBuf, bigBuf, length);
	if (manager->isSpelledRight(upperBuf, length)) {
		manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
		return 0; // correct if converted to all upper case
	}
	unPrecompose(bigBuf, &length, upperBuf, length);
	if (manager->isSpelledRight(bigBuf, length)) {
		manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
		return 0; // correct if precomposed characters expanded, all upper
	}
	if (manager->isSpelledRightMultiple(bigBuf, length)) {
		manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
		return 0; // correct as two words.  Not right for all languages.
	}
	return 1;
}

static char **
uspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	class uSpell *manager;
	
	char **sugg_arr = NULL;
	const utf8_t *sugg;
	wide_t bigBuf[BUFLEN], upperBuf[BUFLEN];
	int length, i;
	utf8_t **list;
	
	manager = (class uSpell *) me->user_data;
	
	list = reinterpret_cast<utf8_t **>(
					   calloc(sizeof(char *), MAXALTERNATIVE));
	length = utf8_wide(bigBuf, reinterpret_cast<const utf8_t *>(word), BUFLEN);
	*out_n_suggs = manager->showAlternatives(bigBuf, length,
						 list, MAXALTERNATIVE);
	
	if (*out_n_suggs)
		{
			sugg_arr = g_new0 (char *, *out_n_suggs + 1);
			for (i = 0; i < *out_n_suggs; i++)
				{
					sugg = list[i];
					if (sugg)
						sugg_arr[i] =
							g_strdup (reinterpret_cast<const gchar *>(sugg));
					free(list[i]);
				}
		}
	free(list);
	return sugg_arr;
}

static void
uspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	class uSpell *manager;
	
	// Don't worry about saving this to a personal dictionary - just add to session
	manager = (class uSpell *) me->user_data;
	manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
}

static void
uspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	class uSpell *manager;
	
	manager = (class uSpell *) me->user_data;
	manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
}

static void
uspell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	g_strfreev (str_list);
}

typedef struct {
	char * language_tag;
	char * corresponding_uspell_file_name;
} Mapping;

static const Mapping mapping [] = {
	{"yi", "yiddish"}
};

static const size_t n_mappings = (sizeof(mapping)/sizeof(mapping[0]));

static class uSpell *
uspell_request_dict (const char * base, const char * mapping)
{
	char * fileName, *transName;

	class uSpell *manager;

	filename = g_build_filename (base, mapping, ".uspell.dat");
	transName = g_build_filename (base, mapping, ".uspell.trans");

	manager = new uSpell(fileName, transName, uSpell::expandPrecomposed);

	g_free (filename);
	g_free (transName);

	return manager;
}

static EnchantDict *
uspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	class uSpell *manager;
	int mapIndex;

	for (mapIndex = 0; mapIndex < n_mappings; mapIndex++) {
		if (!strcmp(tag, mapping[mapIndex].language_tag)) 
			break;
	}
	if (mapIndex == n_mappings)
		return NULL; // I don't understand this language

	char * private_dir = g_build_filename (g_get_home_dir(), ".enchant", "uspell", NULL);
	manager = uspell_request_dict (private_dir, mapping[mapIndex].corresponding_uspell_file_name);
	g_free (private_dir);

	if (!manager)
		manager = uspell_request_dict (ENCHANT_USPELL_DICT_DIR, mapping[mapIndex].corresponding_uspell_file_name);
	if (!manager) 
		return NULL;

	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) manager;
	dict->check = uspell_dict_check;
	dict->suggest = uspell_dict_suggest;
	dict->add_to_personal = uspell_dict_add_to_personal;
	dict->add_to_session = uspell_dict_add_to_session;
	dict->store_replacement = 0;
	dict->free_suggestions = uspell_dict_free_suggestions;
	
	return dict;
}

static void
uspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	class uSpell *manager;
	
	manager = (class uSpell *) dict->user_data;

	//manager->~uSpell();
	delete manager;

	g_free (dict);
}

static void
uspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = uspell_provider_dispose;
	provider->request_dict = uspell_provider_request_dict;
	provider->dispose_dict = uspell_provider_dispose_dict;
	
	return provider;
}

#ifdef __cplusplus
}
#endif
