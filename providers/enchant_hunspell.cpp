/* enchant
 * Copyright (C) 2003-2004 Joan Moratinos <jmo@softcatala.org>, Dom Lachowicz
 * Copyright (C) 2016-2024 Reuben Thomas
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
 * This is the Hunspell Enchant Backend.
 * Hunspell is by László Németh. See: https://hunspell.github.io/
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "enchant-provider.h"

#include <hunspell/hunspell.hxx>
// hunspell itself uses this definition (which only supports the BMP)
#define MAXWORDUTF8LEN (MAXWORDLEN * 3)

#include <glib.h>

/***************************************************************************/

static const char *empty_string = "";

static char *do_iconv(GIConv conv, const char *word) {
	// g_iconv() does not declare its 'in' parameter const, but iconv() does.
	char *in = const_cast<char *>(word);
	size_t len_in = strlen(in);
	size_t len_out = len_in * 3;
	char *out_buf = g_new0(char, len_out + 1);
	char *out = out_buf;
	size_t result = g_iconv(conv, &in, &len_in, &out, &len_out);
	if (static_cast<size_t>(-1) == result)
		return nullptr;
	*out = '\0';
	return out_buf;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class HunspellChecker
{
public:
	HunspellChecker(EnchantProvider *meInit);
	~HunspellChecker();

	bool checkWord (const char *word, size_t len);
	char **suggestWord (const char* const word, size_t len, size_t *out_n_suggs);
	void add (const char* const word, size_t len);
	void remove (const char* const word, size_t len);
	const char *getWordchars ();
	bool apostropheIsWordChar;

	bool requestDictionary (const char * szLang);

private:
	GIConv  m_translate_in; /* Selected translation from/to Unicode */
	GIConv  m_translate_out;
	Hunspell *hunspell;
	EnchantProvider *me;
	char *wordchars; /* Value returned by getWordChars() */
	char *normalizeUtf8(const char *utf8Word, size_t len);
};
#pragma GCC diagnostic pop

/***************************************************************************/

static bool
g_iconv_is_valid(GIConv i)
{
	return (i != nullptr);
}

HunspellChecker::HunspellChecker(EnchantProvider *meInit)
: apostropheIsWordChar(false), m_translate_in(nullptr), m_translate_out(nullptr), hunspell(nullptr), me(meInit), wordchars(nullptr)
{
}

HunspellChecker::~HunspellChecker()
{
	delete hunspell;
	if (g_iconv_is_valid (m_translate_in))
		g_iconv_close(m_translate_in);
	if (g_iconv_is_valid(m_translate_out))
		g_iconv_close(m_translate_out);
	free(wordchars);
}

char*
HunspellChecker::normalizeUtf8(const char *utf8Word, size_t len)
{
	if (len > MAXWORDUTF8LEN
		|| !g_iconv_is_valid(m_translate_in))
		return NULL;

	// the 8bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	char *out = do_iconv(m_translate_in, normalizedWord);
	g_free(normalizedWord);
	return out;
}

bool
HunspellChecker::checkWord(const char *utf8Word, size_t len)
{
	char *out = normalizeUtf8(utf8Word, len);
	if (out == NULL)
		return false;
	bool result = hunspell->spell(std::string(out)) != 0;
	free(out);
	return result;
}

char**
HunspellChecker::suggestWord(const char* const utf8Word, size_t len, size_t *nsug)
{
	if (!g_iconv_is_valid(m_translate_out))
		return nullptr;

	char *out = normalizeUtf8(utf8Word, len);
	if (out == NULL)
		return nullptr;

	std::vector<std::string> sugMS = hunspell->suggest(out);
	g_free(out);
	*nsug = sugMS.size();
	if (*nsug > 0) {
		char **sug = g_new0 (char *, *nsug + 1);
		for (size_t i=0, j=0; i<*nsug; i++) {
			const char *in = sugMS[i].c_str();
			out = do_iconv(m_translate_out, in);
			if (out != NULL)
				sug[j++] = out;
		}
		return sug;
	}
	return nullptr;
}

void
HunspellChecker::add(const char* const utf8Word, size_t len)
{
	char *out = normalizeUtf8(utf8Word, len);
	if (out == NULL)
		return;

	hunspell->add(out);
	free(out);
}

void
HunspellChecker::remove(const char* const utf8Word, size_t len)
{
	char *out = normalizeUtf8(utf8Word, len);
	if (out == NULL)
		return;

	hunspell->remove(out);
	free(out);
}

_GL_ATTRIBUTE_PURE const char*
HunspellChecker::getWordchars()
{
	return static_cast<const char *>(wordchars);
}

static void
s_buildDictionaryDirs (EnchantProvider * me, std::vector<std::string> & dirs)
{
	dirs.clear ();

	gchar * tmp = enchant_provider_get_user_dict_dir (me);
	dirs.push_back (tmp);
	g_free(tmp);

	for (const gchar* const * iter = g_get_system_data_dirs (); *iter; iter++)
		{
			tmp = g_build_filename (*iter, me->identify (me), nullptr);
			dirs.push_back (tmp);
			g_free(tmp);
		}
}

static void
s_buildHashNames (EnchantProvider * me, std::vector<std::string> & names, const char * dict)
{
	names.clear ();

	std::vector<std::string> dirs;
	s_buildDictionaryDirs (me, dirs);

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
hunspell_request_dictionary (EnchantProvider * me, const char * tag)
{
	std::vector<std::string> names;

	s_buildHashNames (me, names, tag);

	for (size_t i = 0; i < names.size (); i++) {
		if (s_fileExists(names[i]) && s_fileExists(s_correspondingAffFile(names[i]))) {
			return strdup (names[i].c_str());
		}
	}

	std::vector<std::string> dirs;
	s_buildDictionaryDirs (me, dirs);

	for (size_t i = 0; i < dirs.size(); i++) {
		GError *err = NULL;
		GDir *dir = g_dir_open (dirs[i].c_str(), 0, &err);
		g_assert ((dir == NULL && err != NULL) || (dir != NULL && err == NULL));
		if (err == NULL) {
			const char *dir_entry;
			while ((dir_entry = g_dir_read_name (dir)) != NULL) {
				if (is_plausible_dict_for_tag(dir_entry, tag)) {
					char *dict = g_build_filename (dirs[i].c_str(),
								       dir_entry, nullptr);
					if(s_fileExists(s_correspondingAffFile(dict))) {
						g_dir_close (dir);
						return dict;
					} else {
						g_debug ("hunspell provider: dictionary file %s has no corresponding affix file", dict);
					}
					g_free(dict);
				}
			}

			g_dir_close (dir);
		} else {
			g_debug ("hunspell provider: could not open directory %s: %s", dirs[i].c_str(), err->message);
			g_error_free (err);
		}
	}

	return NULL;
}

bool
HunspellChecker::requestDictionary(const char *szLang)
{
	char *dic = hunspell_request_dictionary (me, szLang);
	if (!dic)
		return false;

	std::string aff(s_correspondingAffFile(dic));
	if (s_fileExists(aff))
	{
		if (hunspell) {
			delete hunspell;
			free(wordchars);
			wordchars = NULL;
		}
		hunspell = new Hunspell(aff.c_str(), dic);
	}
	free(dic);
	if(hunspell == NULL){
		return false;
	}
	const char *enc = hunspell->get_dic_encoding();

	m_translate_in = g_iconv_open(enc, "UTF-8");
	m_translate_out = g_iconv_open("UTF-8", enc);

	wordchars = do_iconv(m_translate_out, hunspell->get_wordchars());
	if (wordchars == NULL)
		wordchars = strdup(empty_string);
	if (wordchars == NULL)
		return false;
	apostropheIsWordChar = g_utf8_strchr(wordchars, -1, g_utf8_get_char("'")) ||
		g_utf8_strchr(wordchars, -1, g_utf8_get_char("’"));

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

static void
hunspell_dict_add_to_session (EnchantDict * me,
			      const char *const word, size_t len)
{
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	checker->add(word, len);
}

static void
hunspell_dict_remove_from_session (EnchantDict * me,
				   const char *const word, size_t len)
{
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	checker->remove(word, len);
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
	HunspellChecker * checker = static_cast<HunspellChecker *>(me->user_data);
	/* Accept quote marks anywhere except at the end of a word, as per
	   hunspell's textparser.cxx/TextParser::next_token */
	if ((uc == g_utf8_get_char("'") || uc == g_utf8_get_char("’")) && checker->apostropheIsWordChar) {
		return n < 2;
	}
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
hunspell_provider_list_dicts (EnchantProvider * me, size_t * out_n_dicts)
{
	std::vector<std::string> dict_dirs, dicts;
	char ** dictionary_list = NULL;

	s_buildDictionaryDirs (me, dict_dirs);

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
hunspell_provider_request_dict(EnchantProvider * me, const char *const tag)
{
	HunspellChecker * checker = new HunspellChecker(me);

	if (!checker)
		return NULL;

	if (!checker->requestDictionary(tag)) {
		delete checker;
		return NULL;
	}

	EnchantDict *dict = enchant_broker_new_dict(me->owner);
	dict->user_data = (void *) checker;
	dict->check = hunspell_dict_check;
	dict->suggest = hunspell_dict_suggest;
	dict->add_to_session = hunspell_dict_add_to_session;
	dict->remove_from_session = hunspell_dict_remove_from_session;
	dict->get_extra_word_characters = hunspell_dict_get_extra_word_characters;
	dict->is_word_character = hunspell_dict_is_word_character;

	return dict;
}

static void
hunspell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED, EnchantDict * dict)
{
	HunspellChecker *checker = (HunspellChecker *) dict->user_data;
	delete checker;
}

static int
hunspell_provider_dictionary_exists (EnchantProvider * me,
				     const char *const tag)
{
	std::vector <std::string> names;
	s_buildHashNames (me, names, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (s_fileExists(names[i]) && s_fileExists(s_correspondingAffFile(names[i])))
		{
			return 1;
		}
	}

	return 0;
}

static const char *
hunspell_provider_identify (EnchantProvider * me _GL_UNUSED)
{
	return "hunspell";
}

static const char *
hunspell_provider_describe (EnchantProvider * me _GL_UNUSED)
{
	return "Hunspell Provider";
}

EnchantProvider *init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider = enchant_provider_new ();
	provider->request_dict = hunspell_provider_request_dict;
	provider->dispose_dict = hunspell_provider_dispose_dict;
	provider->dictionary_exists = hunspell_provider_dictionary_exists;
	provider->identify = hunspell_provider_identify;
	provider->describe = hunspell_provider_describe;
	provider->list_dicts = hunspell_provider_list_dicts;

	return provider;
}

} // extern C linkage
