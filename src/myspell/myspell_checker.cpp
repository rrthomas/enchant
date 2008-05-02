/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <string>
#include <vector>

#include "enchant.h"
#include "enchant-provider.h"

/* built against hunspell 1.2.2 on 2008-04-12 */

#ifdef WITH_SYSTEM_MYSPELL
#include <hunspell/hunspell.hxx>
#else
#include "hunspell.hxx"
#endif

ENCHANT_PLUGIN_DECLARE("Myspell")

#define G_ICONV_INVALID (GIConv)-1

#include <glib.h>

/***************************************************************************/

class MySpellChecker
{
public:
	MySpellChecker();
	~MySpellChecker();

	bool checkWord (const char *word, size_t len);
	char **suggestWord (const char* const word, size_t len, size_t *out_n_suggs);

	bool requestDictionary (const char * szLang);

private:
	GIConv  m_translate_in; /* Selected translation from/to Unicode */
	GIConv  m_translate_out;
	Hunspell *myspell;
};

/***************************************************************************/

#if defined(_WIN32)
static WCHAR* GetRegistryValue(HKEY baseKey, const WCHAR * uKeyName, const WCHAR * uKey)
{
  	HKEY hKey;
	unsigned long lType;	
	DWORD dwSize;
	WCHAR* wszValue = NULL;

	if(RegOpenKeyEx(baseKey, uKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			/* Determine size of string */
			if(RegQueryValueEx( hKey, uKey, NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS)
				{
					wszValue = g_new0(WCHAR, dwSize + 1);
					RegQueryValueEx(hKey, uKey, NULL, &lType, (LPBYTE) wszValue, &dwSize);
				}
		}

	return wszValue;
}

static char * 
myspell_checker_get_open_office_dicts_dir(void)
{
    WCHAR* wszDirectory;
    char* open_office_dir, * open_office_dicts_dir;

    /*start by trying current user*/
    wszDirectory = GetRegistryValue (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\soffice.exe", L"Path");
    if(wszDirectory == NULL)
    {
        /*next try local machine*/
        wszDirectory = GetRegistryValue (HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\soffice.exe", L"Path");
    }

    if(wszDirectory == NULL)
    {
        return NULL;
    }

    else {
       	open_office_dir = g_utf16_to_utf8 ((gunichar2*)wszDirectory, -1, NULL, NULL, NULL);
		open_office_dicts_dir = g_build_filename(open_office_dir, "share", "dict", "ooo", NULL);
        g_free(wszDirectory);
        g_free(open_office_dir);
        return open_office_dicts_dir;
    }
}
#endif

static bool
g_iconv_is_valid(GIConv i)
{
	return (i != G_ICONV_INVALID);
}

MySpellChecker::MySpellChecker()
	: m_translate_in(G_ICONV_INVALID), m_translate_out(G_ICONV_INVALID), myspell(0)
{
}

MySpellChecker::~MySpellChecker()
{
	delete myspell;
	if (g_iconv_is_valid (m_translate_in ))
		g_iconv_close(m_translate_in);
	if (g_iconv_is_valid(m_translate_out))
		g_iconv_close(m_translate_out);
}

bool
MySpellChecker::checkWord(const char *utf8Word, size_t len)
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
	if ((size_t)-1 == result)
		return false;
	*out = '\0';
	if (myspell->spell(word8))
		return true;
	else
		return false;
}

char**
MySpellChecker::suggestWord(const char* const utf8Word, size_t len, size_t *nsug)
{
	if (len > MAXWORDLEN 
		|| !g_iconv_is_valid(m_translate_in)
		|| !g_iconv_is_valid(m_translate_out))
		return 0;

	// the 8bit encodings use precomposed forms
	char *normalizedWord = g_utf8_normalize (utf8Word, len, G_NORMALIZE_NFC);
	char *in = normalizedWord;
	char word8[MAXWORDLEN + 1];
	char *out = word8;
	size_t len_in = strlen(in);
	size_t len_out = sizeof(word8) - 1;
	size_t result = g_iconv(m_translate_in, &in, &len_in, &out, &len_out);
	g_free(normalizedWord);
	if ((size_t)-1 == result)
		return NULL;

	*out = '\0';
	char **sugMS;
	*nsug = myspell->suggest(&sugMS, word8);
	if (*nsug > 0) {
		char **sug = g_new0 (char *, *nsug + 1);
		for (size_t i=0; i<*nsug; i++) {
			in = sugMS[i];
			len_in = strlen(in);
			len_out = MAXWORDLEN;
			char *word = g_new0(char, len_out + 1);
			out = reinterpret_cast<char *>(word);
			if ((size_t)-1 == g_iconv(m_translate_out, &in, &len_in, &out, &len_out)) {
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
		return 0;
}

static GSList *
myspell_checker_get_dictionary_dirs (void)
{
	GSList *dirs = NULL;

	{
		GSList *config_dirs, *iter;

		config_dirs = enchant_get_user_config_dirs ();
		
		for (iter = config_dirs; iter; iter = iter->next)
			{
				dirs = g_slist_append (dirs, g_build_filename ((const gchar *)iter->data, 
									       "myspell", NULL));
			}

		g_slist_foreach (config_dirs, (GFunc)g_free, NULL);
		g_slist_free (config_dirs);
	}

	/* until I work out how to link the modules against enchant in MacOSX - fjf
	 */
#ifndef XP_TARGET_COCOA
	char * myspell_prefix = NULL;

	/* Look for explicitly set registry values */
	myspell_prefix = enchant_get_registry_value ("Myspell", "Data_Dir");
	if (myspell_prefix)
		dirs = g_slist_append (dirs, myspell_prefix);

	/* Dynamically locate library and search for modules relative to it. */
	char * enchant_prefix = enchant_get_prefix_dir();
	if(enchant_prefix)
		{
			myspell_prefix = g_build_filename(enchant_prefix, "share", "enchant", "myspell", NULL);
			g_free(enchant_prefix);
			dirs = g_slist_append (dirs, myspell_prefix);
		}
#endif

#ifdef ENCHANT_MYSPELL_DICT_DIR
	dirs = g_slist_append (dirs, g_strdup (ENCHANT_MYSPELL_DICT_DIR));
#endif

#if defined(_WIN32)
	char* open_office_dicts_dir = myspell_checker_get_open_office_dicts_dir ();
	if (open_office_dicts_dir) 
        {
		dirs = g_slist_append (dirs, open_office_dicts_dir);
	}
#endif

	return dirs;
}

static void
s_buildDictionaryDirs (std::vector<std::string> & dirs)
{
	GSList *myspell_dirs, *iter;

	dirs.clear ();

	myspell_dirs = myspell_checker_get_dictionary_dirs ();
	for (iter = myspell_dirs; iter; iter = iter->next)
		{
			dirs.push_back ((const char *)iter->data);
		}

	g_slist_foreach (myspell_dirs, (GFunc)g_free, NULL);
	g_slist_free (myspell_dirs);
}

static void
s_buildHashNames (std::vector<std::string> & names, const char * dict)
{
	names.clear ();

	std::vector<std::string> dirs;
	s_buildDictionaryDirs (dirs);

	char *dict_dic = g_strconcat(dict, ".dic", NULL);
	for (size_t i = 0; i < dirs.size(); i++)
		{
			char *tmp = g_build_filename (dirs[i].c_str(), dict_dic, NULL);
			names.push_back (tmp);
			g_free (tmp);
		}

	g_free(dict_dic);
}

static bool
s_hasCorrespondingAffFile(const std::string & dicFile)
{
    std::string aff = dicFile;
    aff.replace(aff.end()-3,aff.end(), "aff");
    return g_file_test(aff.c_str(), G_FILE_TEST_EXISTS) != 0;
}

static char *
myspell_request_dictionary (const char * tag) 
{
	std::vector<std::string> names;

	s_buildHashNames (names, tag);

	for (size_t i = 0; i < names.size (); i++) {
        if (g_file_test(names[i].c_str(), G_FILE_TEST_EXISTS)) {
            if(s_hasCorrespondingAffFile(names[i])){
			    return g_strdup (names[i].c_str());
            }
        }
	}
	
	std::vector<std::string> dirs;
	s_buildDictionaryDirs (dirs);

	for (size_t i = 0; i < dirs.size(); i++) {
		GDir *dir = g_dir_open (dirs[i].c_str(), 0, NULL);
		if (dir) {
			const char *dir_entry;
			while ((dir_entry = g_dir_read_name (dir)) != NULL) {
				if (strncmp (dir_entry, tag, strlen(tag)) == 0 &&
				    strstr (dir_entry, ".dic") != NULL) {
					char *dict = g_build_filename (dirs[i].c_str(), 
								       dir_entry, NULL);
					g_dir_close (dir);
                    if(s_hasCorrespondingAffFile(dict)){
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
MySpellChecker::requestDictionary(const char *szLang)
{
	char *dic = NULL, *aff = NULL;

	dic = myspell_request_dictionary (szLang);
	if (!dic)
		return false;

	aff = g_strdup(dic);
	int len_dic = strlen(dic);
	strcpy(aff+len_dic-3, "aff");
	if (g_file_test(aff, G_FILE_TEST_EXISTS))
	{
		myspell = new Hunspell(aff, dic);
	}
	g_free(dic);
	g_free(aff);
	if(myspell == NULL){
		return false;
	}
	char *enc = myspell->get_dic_encoding();

	m_translate_in = g_iconv_open(enc, "UTF-8");
	m_translate_out = g_iconv_open("UTF-8", enc);

	return true;
}

/*
 * Enchant
 */

static char **
myspell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	MySpellChecker * checker;
	
	checker = (MySpellChecker *) me->user_data;
	return checker->suggestWord (word, len, out_n_suggs);
}

static int
myspell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	MySpellChecker * checker;
	
	checker = (MySpellChecker *) me->user_data;
	
	if (checker->checkWord(word, len))
		return 0;
	
	return 1;
}

static void
myspell_provider_enum_dicts (const char * const directory,
			     std::vector<std::string> & out_dicts)
{
	GDir * dir = g_dir_open (directory, 0, NULL);
	if (dir) {
		const char * entry;
		
		while ((entry = g_dir_read_name (dir)) != NULL) {
			char * utf8_entry = g_filename_to_utf8 (entry, -1, NULL, NULL, NULL);
			if (utf8_entry) {
				std::string dir_entry (utf8_entry);
				g_free (utf8_entry);

				int hit = dir_entry.rfind (".dic");
				if (hit != -1) {
					/* don't include hyphenation dictionaries
					   and require .aff file to be present*/
					if(dir_entry.compare (0, 5, "hyph_") != 0)
					{
						std::string name(dir_entry.substr (0, hit));
						std::string affFileName(name + ".aff");
						char * aff = g_build_filename(directory, affFileName.c_str(), NULL);
						if (g_file_test(aff, G_FILE_TEST_EXISTS))
						{
							out_dicts.push_back (dir_entry.substr (0, hit));
						}
						g_free(aff);
					}
				}
			}
		}

		g_dir_close (dir);
	}
}

static char ** 
myspell_provider_list_dicts (EnchantProvider * me, 
			    size_t * out_n_dicts)
{
	std::vector<std::string> dict_dirs, dicts;
	char ** dictionary_list = NULL;

	s_buildDictionaryDirs (dict_dirs);

	for (size_t i = 0; i < dict_dirs.size(); i++)
		{
			myspell_provider_enum_dicts (dict_dirs[i].c_str(), dicts);
		}

	if (dicts.size () > 0) {
		dictionary_list = g_new0 (char *, dicts.size() + 1);

		for (size_t i = 0; i < dicts.size(); i++)
			dictionary_list[i] = g_strdup (dicts[i].c_str());
	}

	*out_n_dicts = dicts.size ();
	return dictionary_list;
}

static void
myspell_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static EnchantDict *
myspell_provider_request_dict(EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	MySpellChecker * checker;
	
	checker = new MySpellChecker();
	
	if (!checker)
		return NULL;
	
	if (!checker->requestDictionary(tag)) {
		delete checker;
		return NULL;
	}
	
	dict = g_new0(EnchantDict, 1);
	dict->user_data = (void *) checker;
	dict->check = myspell_dict_check;
	dict->suggest = myspell_dict_suggest;
	// don't implement personal, session
	
	return dict;
}

static void
myspell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	MySpellChecker *checker;
	
	checker = (MySpellChecker *) dict->user_data;
	delete checker;
	
	g_free (dict);
}

static int
myspell_provider_dictionary_exists (struct str_enchant_provider * me,
				    const char *const tag)
{
	std::vector <std::string> names;

	s_buildHashNames (names, tag);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
		{
			std::string aff(names[i]);
			aff.replace(aff.end() - 3, aff.end(), "aff");
			if (g_file_test(aff.c_str(), G_FILE_TEST_EXISTS))
				return 1;
		}
	}

	return 0;
}

static void
myspell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
myspell_provider_identify (EnchantProvider * me)
{
	return "myspell";
}

static const char *
myspell_provider_describe (EnchantProvider * me)
{
	return "Myspell Provider";
}

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	     init_enchant_provider (void);

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0(EnchantProvider, 1);
	provider->dispose = myspell_provider_dispose;
	provider->request_dict = myspell_provider_request_dict;
	provider->dispose_dict = myspell_provider_dispose_dict;
	provider->dictionary_exists = myspell_provider_dictionary_exists;
	provider->identify = myspell_provider_identify;
	provider->describe = myspell_provider_describe;
	provider->free_string_list = myspell_provider_free_string_list;
	provider->list_dicts = myspell_provider_list_dicts;

	return provider;
}

} // extern C linkage
