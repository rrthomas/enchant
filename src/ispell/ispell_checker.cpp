/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003 Dom Lachowicz
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

#include <vector>

#include "sp_spell.h"
#include "ispell_checker.h"
#include "enchant.h"

#define G_ICONV_INVALID (GIConv)-1

/***************************************************************************/

typedef struct str_ispell_map
{
	char * lang;
	char * dict;
	char * enc;
} IspellMap;

static const IspellMap ispell_map [] = {
	{"ca"    ,"catala.hash"         ,"iso-8859-1" },
	{"ca_ES" ,"catala.hash"         ,"iso-8859-1" },
	{"cs"    ,"czech.hash"          ,"iso-8859-2" },
	{"cs_CZ" ,"czech.hash"          ,"iso-8859-2" },
	{"da"    ,"dansk.hash"          ,"iso-8859-1" },
	{"da_DK" ,"dansk.hash"          ,"iso-8859-1" },
	{"de"    ,"deutsch.hash"        ,"iso-8859-1" },
	{"de_CH" ,"swiss.hash"          ,"iso-8859-1" },
	{"de_AT" ,"deutsch.hash"        ,"iso-8859-1" },
	{"de_DE" ,"deutsch.hash"        ,"iso-8859-1" },
	{"el"    ,"ellhnika.hash"       ,"iso-8859-7" },
	{"el_GR" ,"ellhnika.hash"       ,"iso-8859-7" },
	{"en"    ,"british.hash"        ,"iso-8859-1" },
	{"en_AU" ,"british.hash"        ,"iso-8859-1" },
	{"en_BZ" ,"british.hash"        ,"iso-8859-1" },
	{"en_CA" ,"british.hash"        ,"iso-8859-1" },
	{"en_GB" ,"british.hash"        ,"iso-8859-1" },
	{"en_IE" ,"british.hash"        ,"iso-8859-1" },
	{"en_JM" ,"british.hash"        ,"iso-8859-1" },
	{"en_NZ" ,"british.hash"        ,"iso-8859-1" },
	{"en_TT" ,"british.hash"        ,"iso-8859-1" },
	{"en_ZA" ,"british.hash"        ,"iso-8859-1" },
	{"en_ZW" ,"british.hash"        ,"iso-8859-1" },
	{"en_PH" ,"american.hash"       ,"iso-8859-1" },
	{"en_US" ,"american.hash"       ,"iso-8859-1" },
	{"eo"    ,"esperanto.hash"      ,"iso-8859-3" },
	{"es"    ,"espanol.hash"        ,"iso-8859-1" },
	{"es_AR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_BO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CL" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_CR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_DO" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_EC" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_ES" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_GT" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_HN" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_MX" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_NI" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PA" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PE" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PR" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_PY" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_SV" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_UY" ,"espanol.hash"        ,"iso-8859-1" },
	{"es_VE" ,"espanol.hash"        ,"iso-8859-1" },
	{"fi"    ,"finnish.hash"        ,"iso-8859-1" },
	{"fi_FI" ,"finnish.hash"        ,"iso-8859-1" },
	{"fr"    ,"francais.hash"       ,"iso-8859-1" },
	{"fr_BE" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_CA" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_CH" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_FR" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_LU" ,"francais.hash"       ,"iso-8859-1" },
	{"fr_MC" ,"francais.hash"       ,"iso-8859-1" },
	{"hu"    ,"hungarian.hash"      ,"iso-8859-2" },
	{"hu_HU" ,"hungarian.hash"      ,"iso-8859-2" },
	{"ga"    ,"irish.hash"          ,"iso-8859-1" },
	{"ga_IE" ,"irish.hash"          ,"iso-8859-1" },
	{"gl"    ,"galician.hash"       ,"iso-8859-1" },
	{"gl_ES" ,"galician.hash"       ,"iso-8859-1" },
	{"it"    ,"italian.hash"        ,"iso-8859-1" },
	{"it_IT" ,"italian.hash"        ,"iso-8859-1" },
	{"it_CH" ,"italian.hash"        ,"iso-8859-1" },
	{"la"    ,"mlatin.hash"         ,"iso-8859-1" },
	{"la_IT" ,"mlatin.hash"         ,"iso-8859-1" },
	{"lt"    ,"lietuviu.hash"       ,"iso-8859-13" },
	{"lt_LT" ,"lietuviu.hash"       ,"iso-8859-13" },
	{"nl"    ,"nederlands.hash"     ,"iso-8859-1" },
	{"nl_NL" ,"nederlands.hash"     ,"iso-8859-1" },
	{"nl_BE" ,"nederlands.hash"     ,"iso-8859-1" },
	{"nb"    ,"norsk.hash"          ,"iso-8859-1" },
	{"nb_NO" ,"norsk.hash"          ,"iso-8859-1" },
	{"nn"    ,"nyorsk.hash"         ,"iso-8859-1" },
	{"nn_NO" ,"nyorsk.hash"         ,"iso-8859-1" },
	{"pl"    ,"polish.hash"         ,"iso-8859-2" },
	{"pl_PL" ,"polish.hash"         ,"iso-8859-2" },
	{"pt"    ,"brazilian.hash"      ,"iso-8859-1" },
	{"pt_BR" ,"brazilian.hash"      ,"iso-8859-1" },
	{"pt_PT" ,"portugues.hash"      ,"iso-8859-1" },
	{"ru"    ,"russian.hash"        ,"koi8-r" },
	{"ru_MD" ,"russian.hash"        ,"koi8-r" },
	{"ru_RU" ,"russian.hash"        ,"koi8-r" },
	{"sc"    ,"sardinian.hash"      ,"iso-8859-1" },
	{"sc_IT" ,"sardinian.hash"      ,"iso-8859-1" },
	{"sk"    ,"slovak.hash"         ,"iso-8859-2" },
	{"sk_SK" ,"slovak.hash"         ,"iso-8859-2" },
	{"sl"    ,"slovensko.hash"      ,"iso-8859-2" },
	{"sl_SI" ,"slovensko.hash"      ,"iso-8859-2" },
	{"sv"    ,"svenska.hash"        ,"iso-8859-1" },
	{"sv_SE" ,"svenska.hash"        ,"iso-8859-1" },
	{"uk"    ,"ukrainian.hash"      ,"koi8-u" },
	{"uk_UA" ,"ukrainian.hash"      ,"koi8-u" },
	{"yi"    ,"yiddish-yivo.hash"   ,"UTF-8" }
};

static const size_t size_ispell_map = G_N_ELEMENTS(ispell_map);

static bool
g_iconv_is_valid(GIConv i)
{
	return (i != G_ICONV_INVALID);
}

void
ISpellChecker::try_autodetect_charset(const char * const inEncoding)
{
	if (inEncoding && strlen(inEncoding))
		{
			m_translate_in = g_iconv_open(inEncoding, "UTF-8");
			m_translate_out = g_iconv_open("UTF-8", inEncoding);
		}
}

/***************************************************************************/
/***************************************************************************/

ISpellChecker::ISpellChecker()
	: deftflag(-1),
     prefstringchar(-1),
     m_bSuccessfulInit(false),
     m_BC(NULL),
     m_cd(NULL),
     m_cl(NULL),
     m_cm(NULL),
     m_ho(NULL),
     m_nd(NULL),
     m_so(NULL),
     m_se(NULL),
     m_ti(NULL),
     m_te(NULL),
     m_hashstrings(NULL),
     m_hashtbl(NULL),
     m_pflaglist(NULL),
     m_sflaglist(NULL),
     m_chartypes(NULL),
     m_infile(NULL),
     m_outfile(NULL),
     m_askfilename(NULL),
     m_Trynum(0),
     m_translate_in(G_ICONV_INVALID),
     m_translate_out(G_ICONV_INVALID)
{
	memset(m_sflagindex,0,sizeof(m_sflagindex));
	memset(m_pflagindex,0,sizeof(m_pflagindex));
}

ISpellChecker::~ISpellChecker()
{
	lcleanup();
	
	if (g_iconv_is_valid (m_translate_in ))
		g_iconv_close(m_translate_in);
	m_translate_in = G_ICONV_INVALID;
	if (g_iconv_is_valid(m_translate_out))
		g_iconv_close(m_translate_out);
	m_translate_out = G_ICONV_INVALID;
}

bool
ISpellChecker::checkWord(const char * const utf8Word, size_t length)
{	
	ichar_t iWord[INPUTWORDLEN + MAXAFFIXLEN];
	char szWord[INPUTWORDLEN + MAXAFFIXLEN];
	
	if (!m_bSuccessfulInit)
		return false;
	
	if (!utf8Word || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return false;
	
	if (m_personal[utf8Word])
		return true;
	if (m_session[utf8Word])
		return true;

	bool retVal = false;

	if (!g_iconv_is_valid(m_translate_in))
		return false;	
	else
		{
			/* convert to 8bit string and null terminate */
			size_t len_in, len_out;
			char *In = (char *)(utf8Word);
			char *Out = szWord;
			
			len_in = length * sizeof(char);
			len_out = sizeof( szWord ) - 1;
			g_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
			*Out = '\0';
		}
	
	if (!strtoichar(iWord, szWord, sizeof(iWord), 0))
		{
			if (good(iWord, 0, 0, 1, 0) == 1 ||
			    compoundgood(iWord, 1) == 1)
				{
					retVal = true;
				}
		}
	
	return retVal;
}

char **
ISpellChecker::suggestWord(const char * const utf8Word, size_t length,
			   size_t * out_n_suggestions)
{
	ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
	char word8[INPUTWORDLEN + MAXAFFIXLEN];
	int  c;
	
	*out_n_suggestions = 0;

	if (!m_bSuccessfulInit)
		return NULL;
	if (!utf8Word || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return NULL;

	if (!g_iconv_is_valid(m_translate_in))
		return NULL;
	else
		{
			/* convert to 8bit string and null terminate */
			
			size_t len_in, len_out;
			char *In = (char *)(utf8Word);
			char *Out = word8;
			len_in = length;
			len_out = sizeof( word8 ) - 1;
			g_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
			*Out = '\0';
		}
	
	if (!strtoichar(iWord, word8, sizeof(iWord), 0))
		makepossibilities(iWord);
	else
		return NULL;
	
	char **sugg_arr = NULL;
	*out_n_suggestions = m_pcount;
	
	{
		sugg_arr = g_new0 (char *, *out_n_suggestions + 1);

		for (c = 0; c < m_pcount; c++)
			{
				int l = strlen(m_possibilities[c]);
				
				char *utf8Sugg = g_new0(char, l + 1);
				
				if (!g_iconv_is_valid(m_translate_out))
					{
						/* copy to 8bit string and null terminate */
						for (int x = 0; x < l; x++)
							utf8Sugg[x] = static_cast<unsigned char>(m_possibilities[c][x]);
						utf8Sugg[l] = 0;
					}
				else
					{
						/* convert to 32bit string and null terminate */
						
						size_t len_in, len_out;
						char *In = m_possibilities[c];
						char *Out = reinterpret_cast<char *>(utf8Sugg);
						
						len_in = l;
						len_out = (l+1);
						g_iconv(m_translate_out, &In, &len_in, &Out, &len_out);
						*(Out) = 0;
					}
				
				sugg_arr[c] = utf8Sugg;
			}
	}
	
	return sugg_arr;
}

static void
s_buildHashNames (std::vector<std::string> & names, const char * dict)
{
	char * tmp, * private_dir;

	names.clear ();

	private_dir = g_build_filename (g_get_home_dir(), ".enchant", 
					"ispell", NULL);

	tmp = g_build_filename (private_dir, dict, NULL);
	names.push_back (tmp);
	g_free (tmp);

	tmp = g_build_filename (ENCHANT_ISPELL_DICT_DIR, dict, NULL);
	names.push_back (tmp);
	g_free (tmp);

	g_free (private_dir);
}

char *
ISpellChecker::loadDictionary (const char * szdict)
{
	std::vector<std::string> dict_names;

	s_buildHashNames (dict_names, szdict);

	for (size_t i = 0; i < dict_names.size(); i++)
		{
			if (linit(const_cast<char*>(dict_names[i].c_str())) >= 0)
				return g_strdup (dict_names[i].c_str());
		}

	return NULL;
}

/*!
 * Load ispell dictionary hash file for given language.
 *
 * \param szLang -  The language tag ("en-US") we want to use
 * \return The name of the dictionary file
 */
bool
ISpellChecker::loadDictionaryForLanguage ( const char * szLang )
{
	char *hashname = NULL;
	
	char * encoding = NULL;
	char * szFile = NULL;
	
	for (size_t i = 0; i < size_ispell_map; i++)
		{
			const IspellMap * mapping = (const IspellMap *)(&(ispell_map[i]));
			if (!strcmp (szLang, mapping->lang))
				{
					szFile = mapping->dict;
					encoding = mapping->enc;
					break;
				}
		}
	
	if (!szFile || !strlen(szFile))
		return false;

	alloc_ispell_struct();
	
	if (!(hashname = loadDictionary(szFile)))
		return false;
	
	// one of the two above calls succeeded
	setDictionaryEncoding (hashname, encoding);
	g_free (hashname);

	return true;
}

void
ISpellChecker::setDictionaryEncoding( const char * hashname, const char * encoding )
{
	/* Get Hash encoding from XML file. This should always work! */
	try_autodetect_charset(encoding);

	if (g_iconv_is_valid(m_translate_in) && g_iconv_is_valid(m_translate_out))
		return; /* success */
	
	/* Test for UTF-8 first */
	prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
	if (prefstringchar >= 0)
		{
			m_translate_in = g_iconv_open("UTF-8", "UTF-8");
			m_translate_out = g_iconv_open("UTF-8", "UTF-8");
		}
	
	if (g_iconv_is_valid(m_translate_in) && g_iconv_is_valid(m_translate_out))
		return; /* success */
	
	/* Test for "latinN" */
	if (!g_iconv_is_valid(m_translate_in))
		{
			/* Look for "altstringtype" names from latin1 to latin15 */
			for(int n1 = 1; n1 <= 15; n1++)
				{
					char * teststring = g_strdup_printf("latin%u", n1);
					prefstringchar = findfiletype(teststring, 1,
								      deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
					if (prefstringchar >= 0)
						{
							m_translate_in = g_iconv_open(teststring, "UTF-8");
							m_translate_out = g_iconv_open("UTF-8", teststring);
							g_free (teststring);
							break;
						}
					else 
						{
							g_free (teststring);
						}
				}
		}
	
	/* If nothing found, use latin1 */
	if (!g_iconv_is_valid(m_translate_in))
		{
			m_translate_in = g_iconv_open("latin1", "UTF-8");
			m_translate_out = g_iconv_open("UTF-8", "latin1");
		}
}

bool
ISpellChecker::requestDictionary(const char *szLang)
{
	if (!loadDictionaryForLanguage (szLang))
		{
			// handle a shortened version of the language tag: en_US => en
			std::string shortened_dict (szLang);
			size_t uscore_pos;
			
			if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1)) {
				shortened_dict = shortened_dict.substr(0, uscore_pos);
				if (!loadDictionaryForLanguage (shortened_dict.c_str()))
					return false;
			}
		}
	
	m_bSuccessfulInit = true;
	
	if (prefstringchar < 0)
		m_defdupchar = 0;
	else
		m_defdupchar = prefstringchar;
	
	return true;
}

static char **
ispell_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
	ISpellChecker * checker;
	
	checker = (ISpellChecker *) me->user_data;
	return checker->suggestWord (word, len, out_n_suggs);
}

static int
ispell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	ISpellChecker * checker;
	
	checker = (ISpellChecker *) me->user_data;
	
	if (checker->checkWord(word, len))
		return 0;
	
	return 1;
}

static void
ispell_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
	ISpellChecker * checker;
	
	// emulate adding to a personal dictionary via a session-like
	// interface

	checker = (ISpellChecker *) me->user_data;
	checker->addToPersonal(word, len);
}

static void
ispell_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
	ISpellChecker * checker;
	
	// implement a session interface

	checker = (ISpellChecker *) me->user_data;
	checker->addToSession(word, len);
}

static void
ispell_dict_free_suggestions (EnchantDict * me, char **str_list)
{
	g_strfreev (str_list);
}

static EnchantDict *
ispell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	ISpellChecker * checker;
	
	checker = new ISpellChecker ();
	
	if (!checker)
		{
			return NULL;
		}
	
	if (!checker->requestDictionary(tag)) {
		delete checker;
		return NULL;
	}
	
	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) checker;
	dict->check = ispell_dict_check;
	dict->suggest = ispell_dict_suggest;
	dict->add_to_personal = ispell_dict_add_to_personal;
	dict->add_to_session = ispell_dict_add_to_session;
	dict->free_suggestions = ispell_dict_free_suggestions;
	
	return dict;
}

static void
ispell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	ISpellChecker * checker;
	
	checker = (ISpellChecker *) dict->user_data;
	delete checker;
	
	g_free (dict);
}

static EnchantDictStatus
ispell_provider_dictionary_status (struct str_enchant_provider * me,
				   const char *const tag)
{
	// TODO: use g_file_test to test existance
	g_warning ("ispell_provider_dictionary_status stub - unimplemented\n");
	return EDS_UNKNOWN;
}

static void
ispell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static char *
ispell_provider_identify (EnchantProvider * me)
{
	return "ispell";
}

static char *
ispell_provider_describe (EnchantProvider * me)
{
	return "Ispell Provider";
}

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = ispell_provider_dispose;
	provider->request_dict = ispell_provider_request_dict;
	provider->dispose_dict = ispell_provider_dispose_dict;
	provider->dictionary_status = ispell_provider_dictionary_status;
	provider->identify = ispell_provider_identify;
	provider->describe = ispell_provider_describe;

	return provider;
}

} // extern C linkage
