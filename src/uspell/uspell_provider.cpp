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

#include <string>

#include <glib.h>
#include "enchant.h"

#include <uspell/utf8convert.h>
#include <uspell/uniprops.h>
#include <uspell/uspell.h>

static const size_t MAXALTERNATIVE = 20; // we won't return more than this number of suggests

static int
uspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	uSpell *manager;
	wide_t *curBuf, *otherBuf, *tmpBuf;
	int length;
	
	manager = (uSpell *) me->user_data;
	curBuf = reinterpret_cast<wide_t *>(calloc(len+1, sizeof(wide_t)));
	length = utf8_wide(curBuf, reinterpret_cast<const utf8_t *>(word), len+1);
	if (manager->isSpelledRight(curBuf, length)) {
		free(curBuf);
		return 0; // correct the first time
	}
	otherBuf = reinterpret_cast<wide_t *>(calloc(len+1, sizeof(wide_t)));
	if (manager->theFlags & uSpell::upperLower) {
		toUpper(otherBuf, curBuf, length);
		if (manager->isSpelledRight(otherBuf, length)) {
			manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
			free(curBuf);
			free(otherBuf);
			return 0; // correct if converted to all upper case
		}
		tmpBuf = curBuf;
		curBuf = otherBuf;
		otherBuf = tmpBuf;
	}
	if (manager->theFlags & uSpell::hasComposition) {
		unPrecompose(otherBuf, &length, curBuf, length);
		if (manager->isSpelledRight(otherBuf, length)) {
			manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
			free(curBuf);
			free(otherBuf);
			return 0; // correct if precomposed characters expanded, all upper
		}
		tmpBuf = curBuf;
		curBuf = otherBuf;
		otherBuf = tmpBuf;
	}
	if (manager->theFlags & uSpell::hasCompounds) {
		if (manager->isSpelledRightMultiple(curBuf, length)) {
			manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
			free(curBuf);
			free(otherBuf);
			return 0; // correct as two words.  Not right for all languages.
		}
	}
	free(curBuf);
	free(otherBuf);
	return 1;
}

static char **
uspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	uSpell *manager;
	
	char **sugg_arr = NULL;
	const utf8_t *sugg;
	wide_t *curBuf;
	int length, i;
	utf8_t **list;
	
	manager = (uSpell *) me->user_data;
	
	list = reinterpret_cast<utf8_t **>(
					   calloc(sizeof(char *), MAXALTERNATIVE));
	curBuf = reinterpret_cast<wide_t *>(calloc(len+1, sizeof(wide_t)));
	length = utf8_wide(curBuf, reinterpret_cast<const utf8_t *>(word), len+1);
	*out_n_suggs = manager->showAlternatives(curBuf, length,
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
	free(curBuf);
	return sugg_arr;
}

static void
uspell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	uSpell *manager;
	
	// Don't worry about saving this to a personal dictionary - just add to session
	manager = (uSpell *) me->user_data;
	manager->acceptWord(reinterpret_cast<const utf8_t *>(word));
}

static void
uspell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	uSpell *manager;
	
	manager = (uSpell *) me->user_data;
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
	int language_flags;
} Mapping;

static const Mapping mapping [] = {
	{"he",    "hebrew",  0},
	{"he_IL", "hebrew",  0},
	{"yi",    "yiddish", uSpell::hasCompounds | uSpell::hasComposition}
};

static const size_t n_mappings = (sizeof(mapping)/sizeof(mapping[0]));

static uSpell *
uspell_request_dict (const char * base, const char * mapping, const int flags)
{
	char *fileName, *transName, *filePart, *transPart;

	uSpell *manager;

	filePart =  g_strconcat(mapping, ".uspell.dat", NULL);
	transPart =  g_strconcat(mapping, ".uspell.trans", NULL);
	fileName = g_build_filename (base, filePart, NULL);
	transName = g_build_filename (base, transPart, NULL);
	g_free(filePart);	
	g_free(transPart);	

	try {
		manager = new uSpell(fileName, transName, flags);
	} 
	catch (...) {
		manager = NULL;
	}

	g_free (fileName);
	g_free (transName);

	return manager;
}

static uSpell *
uspell_request_manager (const char * private_dir, size_t mapIndex)
{
	uSpell * manager = NULL;

	manager = uspell_request_dict (private_dir,
				       mapping[mapIndex].corresponding_uspell_file_name,
				       mapping[mapIndex].language_flags);

	if (!manager)
		manager = uspell_request_dict (ENCHANT_USPELL_DICT_DIR,
					       mapping[mapIndex].corresponding_uspell_file_name,
					       mapping[mapIndex].language_flags);

	return manager;
}

static EnchantDict *
uspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict = NULL;
	uSpell *manager = NULL;
	int mapIndex;

	char * private_dir = g_build_filename (g_get_home_dir(), ".enchant",
					       "uspell", NULL);

	for (mapIndex = 0; mapIndex < n_mappings; mapIndex++) {
		if (!strcmp(tag, mapping[mapIndex].language_tag)) 
			break;
	}

	if (mapIndex < n_mappings) {
		manager = uspell_request_manager (private_dir, mapIndex);
	}

	if (!manager) {
		// try shortened form: he_IL => he
		std::string shortened_dict (tag);
		size_t uscore_pos;
		
		if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1)) {
			shortened_dict = shortened_dict.substr(0, uscore_pos);

			for (mapIndex = 0; mapIndex < n_mappings; mapIndex++) {
				if (!strcmp(shortened_dict.c_str(), mapping[mapIndex].language_tag)) 
					break;
			}

			if (mapIndex < n_mappings) {
				manager = uspell_request_manager (private_dir, mapIndex);
			}			
		}
	}

	g_free (private_dir);

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

EnchantDictStatus uspell_provider_dictionary_status(struct str_enchant_provider * me, 
						    const char *const tag)
{
	// TODO: a g_file_exists check on the dictionary associated with the tag
	g_warning ("uspell_provider_dictionary_status stub - unimplemented\n");
	return(EDS_UNKNOWN);
}

static void
uspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	uSpell *manager;
	
	manager = (uSpell *) dict->user_data;
	delete manager;

	g_free (dict);
}

static void
uspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static char *
uspell_provider_identify (EnchantProvider * me)
{
	return "uspell";
}

static char *
uspell_provider_describe (EnchantProvider * me)
{
	return "Uspell Provider";
}

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = uspell_provider_dispose;
	provider->request_dict = uspell_provider_request_dict;
	provider->dispose_dict = uspell_provider_dispose_dict;
	provider->dictionary_status = uspell_provider_dictionary_status;
	provider->identify = uspell_provider_identify;
	provider->describe = uspell_provider_describe;
	
	return provider;
}

}

