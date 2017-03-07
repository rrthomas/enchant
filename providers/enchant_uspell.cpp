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
 * Boston, MA 02110-1301, USA.
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

/*
 * Raphael Finkel <raphael@cs.uky.edu> is the primary author of Uspell.
 * See: https://github.com/AbiWord/uspell
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


static const size_t MAXALTERNATIVE = 20; // we won't return more than this number of suggestions
static const size_t MAXCHARS = 100; // maximum number of bytes of utf8 or chars of UCS4 in a word

static GSList *
uspell_checker_get_dictionary_dirs (EnchantBroker * broker)
{
	GSList *dirs = nullptr;

	char * config_dir = enchant_get_user_config_dir ();
	dirs = g_slist_append (dirs, g_build_filename (config_dir, "uspell", nullptr));
	g_free (config_dir);

	{
		const gchar* const * system_data_dirs = g_get_system_data_dirs ();
		const gchar* const * iter;

		for (iter = system_data_dirs; *iter; iter++)
			{
				dirs = g_slist_append (dirs, g_build_filename (*iter, "uspell", "dicts", nullptr));
			}
	}

	/* Dynamically locate library and search for modules relative to it. */
	char * enchant_prefix = enchant_get_prefix_dir();
	if(enchant_prefix)
		{
			char * uspell_prefix = g_build_filename(enchant_prefix, "share", "enchant", "uspell", nullptr);
			g_free(enchant_prefix);
			dirs = g_slist_append (dirs, uspell_prefix);
		}

#ifdef ENCHANT_USPELL_DICT_DIR
	dirs = g_slist_append (dirs, g_strdup (ENCHANT_USPELL_DICT_DIR));
#endif

	return dirs;
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
	int length;
  unsigned int i;
	utf8_t **list;
	
	if (len >= MAXCHARS) // no suggestions; the word is outlandish
		return NULL;
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
} // uspell_dict_suggest

static void
uspell_dict_add_to_session (EnchantDict * me, const char *const word,
		     size_t len)
{
	uSpell *manager;
	wide_t buf[MAXCHARS];
	utf8_t myWord[MAXCHARS];
	int length, index;
	
	manager = reinterpret_cast<uSpell *>(me->user_data);

	manager->acceptWord((const utf8_t *)word);
	if (len >= MAXCHARS)
		return; // too long; can't reasonably convert
	// see if we want to acceptWord(uppercase(myWord))
	if (!(manager->theFlags & uSpell::upperLower)) return; // non-case language
	length = utf8_wide(buf, (const utf8_t *)word, MAXCHARS);
	for (index = 0; index < length; index++) {
		if (g_unichar_isupper(buf[index])) return; // case-sensitive word
		buf[index] = g_unichar_toupper(buf[index]);
	}
	wide_utf8(myWord, MAXCHARS, buf, length);
	manager->acceptWord(myWord);
} // uspell_dict_add_to_session

typedef struct {
	const char * language_tag;
	const char * corresponding_uspell_file_name;
	int language_flags;
} Mapping;

static const Mapping mapping [] = {
	{"he",    "hebrew",  0},
	{"he_IL", "hebrew",  0},
	{"yi",    "yiddish", uSpell::hasComposition},
	{"en_US", "american", uSpell::upperLower},
};

static const size_t n_mappings = (sizeof(mapping)/sizeof(mapping[0]));

static void
s_buildHashNames (std::vector<std::string> & names, EnchantBroker * broker, const char * tag)
{
	names.clear ();

	size_t mapIndex;

	for (mapIndex = 0; mapIndex < n_mappings; mapIndex++) {
		if (!strcmp(tag, mapping[mapIndex].language_tag)) 
			break;
	}

	if (mapIndex < n_mappings) {

		GSList *dirs, *iter;
		char * dict = g_strdup_printf ("%s.uspell.dat",
					       mapping[mapIndex].corresponding_uspell_file_name);

		dirs = uspell_checker_get_dictionary_dirs (broker);

		for (iter = dirs; iter; iter = iter->next)
			{
				char *tmp;

				tmp = g_build_filename ((const char *)iter->data, dict, NULL);
				names.push_back (tmp);
				g_free (tmp);
			}

		g_slist_free_full (dirs, g_free);
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
uspell_request_manager (const char * dir, size_t mapIndex)
{
	uSpell * manager = NULL;

	manager = uspell_request_dict (dir,
				       mapping[mapIndex].corresponding_uspell_file_name,
				       mapping[mapIndex].language_flags);

	if (!manager) return NULL;

	// look for a supplementary private dictionary
	const char *config_dir = g_get_user_config_dir();
	if (config_dir) {
		gchar * auxFileName, * transPart;
		transPart = g_strconcat (mapping[mapIndex].language_tag, ".dic", NULL);
		auxFileName = g_build_filename (config_dir, transPart, NULL);
		g_free (transPart);

		(void) manager->assimilateFile (auxFileName);
		g_free (auxFileName);
	}

	return manager;
}

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	     init_enchant_provider (void);

static EnchantDict *
uspell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict = NULL;
	uSpell *manager = NULL;
	size_t mapIndex;
	bool found = false;

	GSList *dirs, *iter;

	for (mapIndex = 0; mapIndex < n_mappings && !found; mapIndex++) {
		if (!strcmp(tag, mapping[mapIndex].language_tag)) 
			break;
	}

	if (!found)
		return NULL;

	dirs = uspell_checker_get_dictionary_dirs (me->owner);

	for (iter = dirs; iter && !manager; iter = iter->next)
		{
			manager = uspell_request_manager ((const char *)iter->data, mapIndex);
		}
	
	g_slist_free_full (dirs, g_free);

	if (!manager) 
		return NULL;

	dict = g_new0 (EnchantDict, 1);
	dict->user_data = manager;
	dict->check = uspell_dict_check;
	dict->suggest = uspell_dict_suggest;
	dict->add_to_session = uspell_dict_add_to_session;
	// don't use personal, session - let higher level implement that
	
	return dict;
}

static int
uspell_provider_dictionary_exists (struct str_enchant_provider * me, 
				   const char *const tag)
{
	std::vector <std::string> names;

	s_buildHashNames (names, me->owner, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
			return 1;
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

static char **
uspell_provider_list_dictionaries (EnchantProvider * me,
				   size_t * out_n_dicts)
{
	size_t i, nb;

	nb = 0;
	for (i = 0; i < n_mappings; i++)
		if (uspell_provider_dictionary_exists (me, mapping[i].language_tag))
			nb++;
	
	*out_n_dicts = nb;
	if (nb == 0)
		return NULL;

	char ** out_dicts = g_new0 (char *, nb + 1);
	for (i = 0; i < n_mappings; i++)
		if (uspell_provider_dictionary_exists (me, mapping[i].language_tag))
			out_dicts[i] = g_strdup (mapping[i].language_tag);

	return out_dicts;
}

static void
uspell_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static void
uspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
uspell_provider_identify (EnchantProvider * me)
{
	return "uspell";
}

static const char *
uspell_provider_describe (EnchantProvider * me)
{
	return "Uspell Provider";
}

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = uspell_provider_dispose;
	provider->request_dict = uspell_provider_request_dict;
	provider->dispose_dict = uspell_provider_dispose_dict;
	provider->dictionary_exists = uspell_provider_dictionary_exists;
	provider->identify = uspell_provider_identify;
	provider->describe = uspell_provider_describe;
	provider->list_dicts = uspell_provider_list_dictionaries;
	provider->free_string_list = uspell_provider_free_string_list;

	return provider;
}

}

