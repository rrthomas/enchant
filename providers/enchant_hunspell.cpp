/* enchant
 * Copyright (C) 2003-2004 Joan Moratinos <jmo@softcatala.org>, Dom Lachowicz
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
 * This is the Hunspell Enchant Backend.
 * Hunspell is by László Németh. See: http://hunspell.github.io/
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <string>
#include <vector>

#include "enchant-provider.h"
#include "unused-parameter.h"

#include <hunspell/hunspell.hxx>

/* Some versions of hunspell (1.4.x) don't have this defined. */
/* This is the defined value at that point */
#ifndef MAXWORDLEN
#define MAXWORDLEN 176
#endif

#include <glib.h>

/***************************************************************************/

class HunspellChecker
{
public:
	HunspellChecker();
	~HunspellChecker();

	bool checkWord (const char *word, size_t len);
	char **suggestWord (const char* const word, size_t len, size_t *out_n_suggs);
	const char *getWordchars ();

	bool requestDictionary (const char * szLang);

private:
	GIConv  m_translate_in; /* Selected translation from/to Unicode */
	GIConv  m_translate_out;
	Hunspell *hunspell;
};

/***************************************************************************/

static bool
g_iconv_is_valid(GIConv i)
{
	return (i != nullptr);
}

HunspellChecker::HunspellChecker()
: m_translate_in(nullptr), m_translate_out(nullptr), hunspell(nullptr)
{
}

HunspellChecker::~HunspellChecker()
{
	delete hunspell;
	if (g_iconv_is_valid (m_translate_in))
		g_iconv_close(m_translate_in);
	if (g_iconv_is_valid(m_translate_out))
		g_iconv_close(m_translate_out);
}

bool
HunspellChecker::checkWord(const char *utf8Word, size_t len)
{
	if (len > MAXWORDLEN || !g_iconv_is_valid(m_translate_in))
		return false;

	// the 8bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	char *in = normalizedWord;
	char word8[MAXWORDLEN + 1];
	char *out = word8;
	size_t len_in = strlen(in);
	size_t len_out = sizeof( word8 ) - 1;
	size_t result = g_iconv(m_translate_in, &in, &len_in, &out, &len_out);
	g_free(normalizedWord);
	if (static_cast<size_t>(-1) == result)
		return false;
	*out = '\0';
	if (hunspell->spell(word8))
		return true;
	else
		return false;
}

char**
HunspellChecker::suggestWord(const char* const utf8Word, size_t len, size_t *nsug)
{
	if (len > MAXWORDLEN 
		|| !g_iconv_is_valid(m_translate_in)
		|| !g_iconv_is_valid(m_translate_out))
		return nullptr;

	// the 8bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	char *in = normalizedWord;
	char word8[MAXWORDLEN + 1];
	char *out = word8;
	size_t len_in = strlen(in);
	size_t len_out = sizeof(word8) - 1;
	size_t result = g_iconv(m_translate_in, &in, &len_in, &out, &len_out);
	g_free(normalizedWord);
	if (static_cast<size_t>(-1) == result)
		return nullptr;

	*out = '\0';
	char **sugMS;
	*nsug = hunspell->suggest(&sugMS, word8);
	if (*nsug > 0) {
		char **sug = g_new0 (char *, *nsug + 1);
		for (size_t i=0; i<*nsug; i++) {
			in = sugMS[i];
			len_in = strlen(in);
			len_out = MAXWORDLEN;
			char *word = g_new0(char, len_out + 1);
			out = word;
			if (static_cast<size_t>(-1) == g_iconv(m_translate_out, &in, &len_in, &out, &len_out)) {
				for (size_t j = i; j < *nsug; j++)
					free(sugMS[j]);
				free(sugMS);

				*nsug = i;
				return sug;
			}
			*(out) = 0;
			sug[i] = word;
			free(sugMS[i]);
		}
		free(sugMS);
		return sug;
	}
	else
		return nullptr;
}

const char*
HunspellChecker::getWordchars()
{
	return hunspell->get_wordchars();
}

static void
s_buildDictionaryDirs (std::vector<std::string> & dirs)
{
	dirs.clear ();

	char * config_dir = enchant_get_user_config_dir ();
	dirs.push_back (g_build_filename (config_dir, "hunspell", nullptr));
	free (config_dir);

	for (const gchar* const * iter = g_get_system_data_dirs (); *iter; iter++)
		{
			dirs.push_back (g_build_filename (*iter, "hunspell", nullptr));
		}

	/* Dynamically locate library and search for modules relative to it. */
	char * enchant_prefix = enchant_get_prefix_dir();
	if(enchant_prefix)
		{
			dirs.push_back (g_build_filename(enchant_prefix, "share", "enchant", "hunspell", nullptr));
			g_free(enchant_prefix);
		}

#ifdef ENCHANT_HUNSPELL_DICT_DIR
	dirs.push_back (enchant_relocate (ENCHANT_HUNSPELL_DICT_DIR));
#endif
}

static void
s_buildHashNames (std::vector<std::string> & names, const char * dict)
{
	names.clear ();

	std::vector<std::string> dirs;
	s_buildDictionaryDirs (dirs);

	char *dict_dic = g_strconcat(dict, ".dic", nullptr);
	for (size_t i = 0; i < dirs.size(); i++)
		{
			char *tmp = g_build_filename (dirs[i].c_str(), dict_dic, nullptr);
			names.push_back (tmp);
			g_free (tmp);
		}

	g_free(dict_dic);
}

static const std::string
s_correspondingAffFile(const std::string & dicFile)
{
	std::string aff = dicFile;
	aff.replace(aff.end()-3,aff.end(), "aff");
	return aff;
}

static bool
s_fileExists(const std::string & file)
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
    if (strcmp(dir_entry+dir_entry_len-dic_suffix_len, dic_suffix) != 0)
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
hunspell_request_dictionary (const char * tag)
{
	std::vector<std::string> names;

	s_buildHashNames (names, tag);

	for (size_t i = 0; i < names.size (); i++) {
		if (g_file_test(names[i].c_str(), G_FILE_TEST_EXISTS) &&
		    s_fileExists(s_correspondingAffFile(names[i]))) {
			return strdup (names[i].c_str());
		}
	}
	
	std::vector<std::string> dirs;
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
				}
			}

			g_dir_close (dir);
		}
	}

	return NULL;
}

bool
HunspellChecker::requestDictionary(const char *szLang)
{
	char *dic = hunspell_request_dictionary (szLang);
	if (!dic)
		return false;

	std::string aff(s_correspondingAffFile(dic));
	if (s_fileExists(aff))
	{
		hunspell = new Hunspell(aff.c_str(), dic);
	}
	free(dic);
	if(hunspell == NULL){
		return false;
	}
	const char *enc = hunspell->get_dic_encoding();

	m_translate_in = g_iconv_open(enc, "UTF-8");
	m_translate_out = g_iconv_open("UTF-8", enc);

	return true;
}

/*
 * Enchant
 */

static char **
hunspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	return checker->suggestWord (word, len, out_n_suggs);
}

static int
hunspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	
	if (checker->checkWord(word, len))
		return 0;
	
	return 1;
}

static const char*
hunspell_dict_get_extra_word_characters (EnchantDict *me)
{
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	return checker->getWordchars();
}

static int
hunspell_dict_is_word_character (EnchantDict *me, uint32_t uc, size_t n)
{
	(void)n;
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	return g_unichar_isalpha(uc) || g_utf8_strchr(checker->getWordchars(), -1, uc);
}

static void
hunspell_provider_enum_dicts (const char * const directory,
			     std::vector<std::string> & out_dicts)
{
	GDir * dir = g_dir_open (directory, 0, nullptr);
	if (dir) {
		const char * entry;
		while ((entry = g_dir_read_name (dir)) != NULL) {
			char * utf8_entry = g_filename_to_utf8 (entry, -1, nullptr, nullptr, nullptr);
			if (utf8_entry) {
				std::string dir_entry (utf8_entry);
				g_free (utf8_entry);

				int hit = dir_entry.rfind (".dic");
				if (hit != -1) {
					/* don't include hyphenation dictionaries
					   and require .aff file to be present*/
					if(dir_entry.compare (0, 5, "hyph_") != 0)
					{
						char * dic = g_build_filename(directory, dir_entry.c_str(), nullptr);
						if (s_fileExists(s_correspondingAffFile(dic)))
						{
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
hunspell_provider_list_dicts (EnchantProvider * me _GL_UNUSED_PARAMETER, 
			      size_t * out_n_dicts)
{
	std::vector<std::string> dict_dirs, dicts;
	char ** dictionary_list = NULL;

	s_buildDictionaryDirs (dict_dirs);

	for (size_t i = 0; i < dict_dirs.size(); i++)
		{
			hunspell_provider_enum_dicts (dict_dirs[i].c_str(), dicts);
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
hunspell_provider_request_dict(EnchantProvider * me _GL_UNUSED_PARAMETER, const char *const tag)
{
	HunspellChecker * checker = new HunspellChecker();
	
	if (!checker)
		return NULL;
	
	if (!checker->requestDictionary(tag)) {
		delete checker;
		return NULL;
	}
	
	EnchantDict *dict = g_new0(EnchantDict, 1);
	dict->user_data = (void *) checker;
	dict->check = hunspell_dict_check;
	dict->suggest = hunspell_dict_suggest;
	// don't implement personal, session
	dict->get_extra_word_characters = hunspell_dict_get_extra_word_characters;
	dict->is_word_character = hunspell_dict_is_word_character;
	
	return dict;
}

static void
hunspell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED_PARAMETER, EnchantDict * dict)
{
	HunspellChecker *checker = (HunspellChecker *) dict->user_data;
	delete checker;
	
	g_free (dict);
}

static int
hunspell_provider_dictionary_exists (struct str_enchant_provider * me _GL_UNUSED_PARAMETER,
				     const char *const tag)
{
	std::vector <std::string> names;
	s_buildHashNames (names, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS) &&
		    s_fileExists(s_correspondingAffFile(names[i])))
		{
			return 1;
		}
	}

	return 0;
}

static void
hunspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
hunspell_provider_identify (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "hunspell";
}

static const char *
hunspell_provider_describe (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
	return "Hunspell Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = g_new0(EnchantProvider, 1);
	provider->dispose = hunspell_provider_dispose;
	provider->request_dict = hunspell_provider_request_dict;
	provider->dispose_dict = hunspell_provider_dispose_dict;
	provider->dictionary_exists = hunspell_provider_dictionary_exists;
	provider->identify = hunspell_provider_identify;
	provider->describe = hunspell_provider_describe;
	provider->list_dicts = hunspell_provider_list_dicts;

	return provider;
}

} // extern C linkage
