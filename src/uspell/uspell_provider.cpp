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

#include <string>
#include <vector>

#include <glib.h>

#include "enchant.h"
#include "enchant-provider.h"

#include <uspell/utf8convert.h>
#include <uspell/uniprops.h>
#include <uspell/uspell.h>

ENCHANT_PLUGIN_DECLARE("Uspell")

static const size_t MAXALTERNATIVE = 20; // we won't return more than this number of suggestions
static const size_t MAXCHARS = 100; // maximum number of bytes of utf8 or chars of UCS4 in a word

static char *
uspell_checker_get_prefix (void)
{
	char * data_dir = NULL;

	data_dir = enchant_get_registry_value ("Uspell", "Data_Dir");
	if (data_dir)
		return data_dir;

#ifdef ENCHANT_USPELL_DICT_DIR
	return g_strdup (ENCHANT_USPELL_DICT_DIR);
#else
	return NULL;
#endif
}

static int
uspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	uSpell *manager;
	wide_t buf1[MAXCHARS], buf2[MAXCHARS], *curBuf, *otherBuf, *tmpBuf;
	utf8_t myWord[MAXCHARS];
	int length;
	
	if (len >= MAXCHARS)
		return 1; // too long; can't be right
	memcpy(reinterpret_cast<char *>(myWord), word, len);
	myWord[len] = 0;
	curBuf = buf1;
	otherBuf = buf2;
	manager = reinterpret_cast<uSpell *>(me->user_data);
#ifdef DEBUG
	fprintf(stdout, "Checking [%s]\n", word);
#endif
	length = utf8_wide(curBuf, myWord, MAXCHARS);
	if (manager->isSpelledRight(curBuf, length)) {
		return 0; // correct the first time
	}
	if (manager->theFlags & uSpell::upperLower) {
		toUpper(otherBuf, curBuf, length);
		if (manager->isSpelledRight(otherBuf, length)) {
			manager->acceptWord(myWord);
			return 0; // correct if converted to all upper case
		}
		tmpBuf = curBuf;
		curBuf = otherBuf;
		otherBuf = tmpBuf;
	}
	if (manager->theFlags & uSpell::hasComposition) {
		unPrecompose(otherBuf, &length, curBuf, length);
		if (manager->isSpelledRight(otherBuf, length)) {
			manager->acceptWord(myWord);
			return 0; // correct if precomposed characters expanded, all upper
		}
		tmpBuf = curBuf;
		curBuf = otherBuf;
		otherBuf = tmpBuf;
	}
	if (manager->theFlags & uSpell::hasCompounds) {
		if (manager->isSpelledRightMultiple(curBuf, length)) {
			manager->acceptWord(myWord);
			return 0; // correct as two words.  Not right for all languages.
		}
	}
	return 1;
}

static char **
uspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	uSpell *manager;
	utf8_t myWord[MAXCHARS];
	
	char **sugg_arr = NULL;
	const utf8_t *sugg;
	wide_t buf[MAXCHARS];
	int length, i;
	utf8_t **list;
	
	if (len >= MAXCHARS) // no suggestions; the word is outlandish
		return g_new0 (char *, 1); 
	memcpy(reinterpret_cast<char *>(myWord), word, len);
	myWord[len] = 0;
	manager = reinterpret_cast<uSpell *>(me->user_data);
	
	list = reinterpret_cast<utf8_t **>(
					   calloc(sizeof(char *), MAXALTERNATIVE));
	length = utf8_wide(buf, myWord, MAXCHARS);
	*out_n_suggs = manager->showAlternatives(buf, length,
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

static void
s_buildHashNames (std::vector<std::string> & names, const char * tag)
{
	names.clear ();

	size_t mapIndex;

	for (mapIndex = 0; mapIndex < n_mappings; mapIndex++) {
		if (!strcmp(tag, mapping[mapIndex].language_tag)) 
			break;
	}

	if (mapIndex < n_mappings) {

		char * tmp, * private_dir, * home_dir, * uspell_prefix;
		
		char * dict = mapping[mapIndex].language_tag;

		home_dir = enchant_get_user_home_dir ();
		
		if (home_dir) {
			private_dir = g_build_filename (home_dir, ".enchant", 
							"uspell", NULL);
			
			tmp = g_build_filename (private_dir, dict, NULL);
			names.push_back (tmp);
			g_free (tmp);
			
			g_free (private_dir);
			g_free (home_dir);
		}
		
		uspell_prefix = uspell_checker_get_prefix ();
		if (uspell_prefix) {
			tmp = g_build_filename (uspell_prefix, dict, NULL);
			names.push_back (tmp);
			g_free (tmp);
			g_free (uspell_prefix);
		}
	}
}

static uSpell *
uspell_request_dict (const char * base, const char * mapping, const int flags)
{
	char *fileName, *transName, *filePart, *transPart;

	uSpell *manager;

	if (!base)
		return NULL;

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
	char * uspell_prefix;

	uSpell * manager = NULL;

	manager = uspell_request_dict (private_dir,
				       mapping[mapIndex].corresponding_uspell_file_name,
				       mapping[mapIndex].language_flags);

	if (!manager) {
		uspell_prefix = uspell_checker_get_prefix ();

		if (uspell_prefix) {
			manager = uspell_request_dict (uspell_prefix,
						       mapping[mapIndex].corresponding_uspell_file_name,
						       mapping[mapIndex].language_flags);
			g_free (uspell_prefix);
		}
	}

	return manager;
}

static EnchantDict *
uspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict = NULL;
	uSpell *manager = NULL;
	int mapIndex;

	char * private_dir = NULL, * home_dir;

	home_dir = enchant_get_user_home_dir ();

	if (home_dir) {
		private_dir = g_build_filename (home_dir, ".enchant",
						"uspell", NULL);
		g_free (home_dir);
	}

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
	dict->user_data = manager;
	dict->check = uspell_dict_check;
	dict->suggest = uspell_dict_suggest;
	dict->store_replacement = 0;
	dict->free_suggestions = uspell_dict_free_suggestions;
	// don't use personal, session - let higher level implement that
	
	return dict;
}

static int
uspell_provider_dictionary_status(struct str_enchant_provider * me, 
				  const char *const tag)
{
	std::vector <std::string> names;

	s_buildHashNames (names, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
			return 1;
	}

	std::string shortened_dict (tag);
	size_t uscore_pos;
	
	if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1)) {
		shortened_dict = shortened_dict.substr(0, uscore_pos);

		s_buildHashNames (names, shortened_dict.c_str());
		for (size_t i = 0; i < names.size(); i++) {
			if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
				return 1;
		}
	}

	return 0;
}

static void
uspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	uSpell *manager = reinterpret_cast<uSpell *>(dict->user_data);
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

