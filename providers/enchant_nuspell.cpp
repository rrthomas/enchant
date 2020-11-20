/* enchant
 * Copyright (C) 2020 Sander van Geloven
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * This is the Nuspell Enchant Backend.
 * Nuspell is by Dimitrij Mijoski and Sander van Geloven.
 * See: http://nuspell.github.io/
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "enchant-provider.h"
#include "unused-parameter.h"

#include <nuspell/dictionary.hxx>
#include <nuspell/finder.hxx>

#include <glib.h>

using namespace std;
using namespace nuspell;

/***************************************************************************/

class NuspellChecker
{
public:
	bool checkWord (const char *word, size_t len);
	char **suggestWord (const char* const word, size_t len, size_t *out_n_suggs);

	bool requestDictionary (const char * szLang);

private:
	Dictionary nuspell;
};

/***************************************************************************/

bool
NuspellChecker::checkWord(const char *utf8Word, size_t len)
{
	// the 8-bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	auto ret = nuspell.spell(normalizedWord);
	g_free(normalizedWord);
	return ret;
}

char**
NuspellChecker::suggestWord(const char* const utf8Word, size_t len, size_t *nsug)
{
	// the 8-bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	auto suggestions = vector<string>();
	nuspell.suggest(normalizedWord, suggestions);
	g_free(normalizedWord);
	if (suggestions.empty())
		return nullptr;
	*nsug = suggestions.size();
	char **sug = g_new0 (char *, *nsug + 1);
	size_t i = 0;
	for (auto& suggest : suggestions) {
		char *word = g_new0(char, suggest.size() + 1);
		strcpy(word, suggest.c_str());
		sug[i] = word;
		i++;
	}
	return sug;
}

static void
s_buildDictionaryDirs (vector<string> & dirs)
{
	dirs.clear ();

	/* 1. personal overrides for Enchant
	 *    ~/.config/enchant/nuspell
	 */
	gchar * tmp;
	char * config_dir = enchant_get_user_config_dir ();
	tmp = g_build_filename (config_dir, "nuspell", nullptr);
	dirs.push_back (tmp);
	free (config_dir);
	g_free(tmp);

	/* Dynamically retrieved from Nuspell dictionary finder:
	 * 2. personal overrides for Hunspell
	 *    $XDG_DATA_HOME/hunspell
	 *    $XDG_DATA_HOME by default is $HOME/.local/share/
	 * 3. system installed for Hunspell
	 *    $XDG_DATA_DIRS/hunspell
	 *    $XDG_DATA_DIRS/myspell (needed for Fedora)
	 *    $XDG_DATA_DIRS by default are /usr/local/share and /usr/share
	 */
	nuspell::append_default_dir_paths(dirs);

	/* 5. system installations by Enchant
	 *    /usr/local/share/enchant/nuspell
	 *    /usr/share/enchant/nuspell
	 */
	char * enchant_prefix = enchant_get_prefix_dir();
	if (enchant_prefix) {
		tmp = g_build_filename(enchant_prefix, "share", "enchant", "nuspell", nullptr);
		dirs.push_back (tmp);
		g_free(enchant_prefix);
		g_free(tmp);
	}

	/* Hunspell paths are used, therefore ENCHANT_NUSPELL_DICT_DIR is
	 * irrelevant. Hence, the following paths are not to be considered:
	 * /usr/local/share/nuspell and /usr/share/nuspell
	 */
}

static void
s_buildHashNames (vector<string> & names, const char * dict)
{
	names.clear ();

	vector<string> dirs;
	s_buildDictionaryDirs (dirs);

	char *dict_dic = g_strconcat(dict, ".dic", nullptr);
	for (size_t i = 0; i < dirs.size(); i++) {
		char *tmp = g_build_filename (dirs[i].c_str(), dict_dic, nullptr);
		names.push_back (tmp);
		g_free (tmp);
	}

	g_free(dict_dic);
}

static const string
s_correspondingAffFile(const string & dicFile)
{
	string aff = dicFile;
	if (aff.size() >= 4 && aff.compare(aff.size() - 4, 4, ".dic") == 0) {
		aff.erase(aff.size() - 3);
		aff += "aff";
	}
	return aff;
}

static bool
s_fileExists(const string & file)
{
	return g_file_test(file.c_str(), G_FILE_TEST_EXISTS) != 0;
}

static bool is_plausible_dict_for_tag(const char *dir_entry, const char *tag)
{
	const char *dic_suffix = ".dic";
	size_t dic_suffix_len = strlen(dic_suffix);
	size_t dir_entry_len = strlen(dir_entry);
	size_t tag_len = strlen(tag);

	if (dir_entry_len - dic_suffix_len < tag_len)
		return false;
	if (strcmp(dir_entry + dir_entry_len - dic_suffix_len, dic_suffix) != 0)
		return false;
	if (strncmp (dir_entry, tag, tag_len) != 0)
		return false;
	//e.g. requested dict for "fi",
	//reject "fil_PH.dic"
	//allow "fi-FOO.dic", "fi_FOO.dic", "fi.dic", etc.
	if (!ispunct(dir_entry[tag_len]))
		return false;
	return true;
}

static char *
nuspell_request_dictionary (const char * tag)
{
	vector<string> names;

	s_buildHashNames (names, tag);

	for (size_t i = 0; i < names.size (); i++) {
		if (g_file_test(names[i].c_str(), G_FILE_TEST_EXISTS) &&
		    s_fileExists(s_correspondingAffFile(names[i]))) {
			return strdup (names[i].c_str());
		}
	}

	vector<string> dirs;
	s_buildDictionaryDirs (dirs);

	for (size_t i = 0; i < dirs.size(); i++) {
		GDir *dir = g_dir_open (dirs[i].c_str(), 0, nullptr);
		if (dir) {
			const char *dir_entry;
			while ((dir_entry = g_dir_read_name (dir)) != NULL) {
				if (is_plausible_dict_for_tag(dir_entry, tag)) {
					char *dict = g_build_filename (dirs[i].c_str(),
								       dir_entry, nullptr);
					if(s_fileExists(s_correspondingAffFile(dict))) {
						g_dir_close (dir);
						return dict;
					}
					g_free(dict);
				}
			}

			g_dir_close (dir);
		}
	}

	return NULL;
}

bool
NuspellChecker::requestDictionary(const char *szLang)
{
	char *dic = nuspell_request_dictionary (szLang);
	if (!dic)
		return false;
	string aff(s_correspondingAffFile(dic));
	if (!s_fileExists(aff))
		return false;
	auto path = string(dic);
	free(dic);
	if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".dic") == 0)
		path.erase(path.size() - 4);
	else
		return false;
	try {
		nuspell = nuspell::Dictionary::load_from_path(path);
	} catch (const std::runtime_error& e) {
		return false;
	}

	return true;
}

/*
 * Enchant
 */

static char **
nuspell_dict_suggest (EnchantDict * me, const char *const word,
		      size_t len, size_t * out_n_suggs)
{
	NuspellChecker * checker = static_cast<NuspellChecker *>(me->user_data);
	return checker->suggestWord (word, len, out_n_suggs);
}

static int
nuspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	NuspellChecker * checker = static_cast<NuspellChecker *>(me->user_data);

	return !(checker->checkWord(word, len));
}

static int
nuspell_dict_is_word_character (EnchantDict * me _GL_UNUSED_PARAMETER,
				uint32_t uc, size_t n _GL_UNUSED_PARAMETER)
{
	return g_unichar_isalpha(uc);
}

static void
nuspell_provider_enum_dicts (const char * const directory,
			     vector<string> & out_dicts)
{
	GDir * dir = g_dir_open (directory, 0, nullptr);
	if (dir) {
		const char * entry;
		while ((entry = g_dir_read_name (dir)) != NULL) {
			char * utf8_entry = g_filename_to_utf8 (entry, -1, nullptr, nullptr, nullptr);
			if (utf8_entry) {
				string dir_entry (utf8_entry);
				g_free (utf8_entry);

				int hit = dir_entry.rfind (".dic");
				// don't include hyphenation dictionaries
				if (hit != -1) {
					// require .aff file to be present
					if(dir_entry.compare (0, 5, "hyph_") != 0) {
						char * dic = g_build_filename(directory, dir_entry.c_str(), nullptr);
						if (s_fileExists(s_correspondingAffFile(dic))) {
							out_dicts.push_back (dir_entry.substr (0, hit));
						}
						g_free(dic);
					}
				}
			}
		}

		g_dir_close (dir);
	}
}

extern "C" {

static char **
nuspell_provider_list_dicts (EnchantProvider * me _GL_UNUSED_PARAMETER,
			     size_t * out_n_dicts)
{
	vector<string> dict_dirs, dicts;
	char ** dictionary_list = NULL;

	s_buildDictionaryDirs (dict_dirs);

	for (size_t i = 0; i < dict_dirs.size(); i++) {
		nuspell_provider_enum_dicts (dict_dirs[i].c_str(), dicts);
	}

	if (dicts.size () > 0) {
		dictionary_list = g_new0 (char *, dicts.size() + 1);

		for (size_t i = 0; i < dicts.size(); i++)
			dictionary_list[i] = g_strdup (dicts[i].c_str());
	}

	*out_n_dicts = dicts.size ();
	return dictionary_list;
}

static EnchantDict *
nuspell_provider_request_dict(EnchantProvider * me _GL_UNUSED_PARAMETER, const char *const tag)
{
	NuspellChecker * checker = new NuspellChecker();

	if (!checker)
		return NULL;

	if (!checker->requestDictionary(tag)) {
		delete checker;
		return NULL;
	}

	EnchantDict *dict = g_new0(EnchantDict, 1);
	dict->user_data = (void *) checker;
	dict->check = nuspell_dict_check;
	dict->suggest = nuspell_dict_suggest;
	// don't implement personal, session
	dict->is_word_character = nuspell_dict_is_word_character;

	return dict;
}

static void
nuspell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED_PARAMETER, EnchantDict * dict)
{
	NuspellChecker *checker = (NuspellChecker *) dict->user_data;
	delete checker;

	g_free (dict);
}

static int
nuspell_provider_dictionary_exists (struct str_enchant_provider * me _GL_UNUSED_PARAMETER,
				    const char *const tag)
{
	vector <string> names;
	s_buildHashNames (names, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS) &&
		    s_fileExists(s_correspondingAffFile(names[i]))) {
			return 1;
		}
	}

	return 0;
}

static void
nuspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
nuspell_provider_identify (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "nuspell";
}

static const char *
nuspell_provider_describe (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "Nuspell Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = g_new0(EnchantProvider, 1);
	provider->dispose = nuspell_provider_dispose;
	provider->request_dict = nuspell_provider_request_dict;
	provider->dispose_dict = nuspell_provider_dispose_dict;
	provider->dictionary_exists = nuspell_provider_dictionary_exists;
	provider->identify = nuspell_provider_identify;
	provider->describe = nuspell_provider_describe;
	provider->list_dicts = nuspell_provider_list_dicts;

	return provider;
}

} // extern C linkage
