/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <string>
#include <vector>
#include <libhyphenate\Hyphenator.h>

#include "enchant-provider.h"
#include "sp_spell.h"
#include "ispell_checker.h"
#include "enchant.h"
using namespace Hyphenate;

#ifndef ENCHANT_ISPELL_HOME_DIR
#define ENCHANT_ISPELL_HOME_DIR "ispell"
#endif

ENCHANT_PLUGIN_DECLARE("Ispell")

#define G_ICONV_INVALID (GIConv)-1

/***************************************************************************/

typedef struct str_ispell_map
{
	const char * lang;
	const char * dict;
	const char * enc;
} IspellMap;

static const IspellMap ispell_map [] = {
	{"ca"    ,"catala.hash"         ,"iso-8859-1" },
	{"cs"    ,"czech.hash"          ,"iso-8859-2" },
	{"da"    ,"dansk.hash"          ,"iso-8859-1" },
	{"de"    ,"deutsch.hash"        ,"iso-8859-1" },
	{"de_CH" ,"swiss.hash"          ,"iso-8859-1" },
	{"el"    ,"ellhnika.hash"       ,"iso-8859-7" },
	{"en"    ,"british.hash"        ,"iso-8859-1" },
	{"en_PH" ,"american.hash"       ,"iso-8859-1" },
	{"en_US" ,"american.hash"       ,"iso-8859-1" },
	{"eo"    ,"esperanto.hash"      ,"iso-8859-3" },
	{"es"    ,"espanol.hash"        ,"iso-8859-1" },
	{"fi"    ,"finnish.hash"        ,"iso-8859-1" },
	{"fr"    ,"francais.hash"       ,"iso-8859-1" },
	{"hu"    ,"hungarian.hash"      ,"iso-8859-2" },
	{"ga"    ,"irish.hash"          ,"iso-8859-1" },
	{"gl"    ,"galician.hash"       ,"iso-8859-1" },
	{"ia"    ,"interlingua.hash"    ,"iso-8859-1" },
	{"it"    ,"italian.hash"        ,"iso-8859-1" },
	{"la"    ,"mlatin.hash"         ,"iso-8859-1" },
	{"lt"    ,"lietuviu.hash"       ,"iso-8859-13" },
	{"nl"    ,"nederlands.hash"     ,"iso-8859-1" },
	{"nb"    ,"norsk.hash"          ,"iso-8859-1" },
	{"nn"    ,"nynorsk.hash"        ,"iso-8859-1" },
	{"no"    ,"norsk.hash"          ,"iso-8859-1" },
	{"pl"    ,"polish.hash"         ,"iso-8859-2" },
	{"pt"    ,"brazilian.hash"      ,"iso-8859-1" },
	{"pt_PT" ,"portugues.hash"      ,"iso-8859-1" },
	{"ru"    ,"russian.hash"        ,"koi8-r" },
	{"sc"    ,"sardinian.hash"      ,"iso-8859-1" },
	{"sk"    ,"slovak.hash"         ,"iso-8859-2" },
	{"sl"    ,"slovensko.hash"      ,"iso-8859-2" },
	{"sv"    ,"svenska.hash"        ,"iso-8859-1" },
	{"uk"    ,"ukrainian.hash"      ,"koi8-u" },
	{"yi"    ,"yiddish-yivo.hash"   ,"utf-8" }
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

ISpellChecker::ISpellChecker(EnchantBroker * broker)
: m_broker(broker),
	deftflag(-1),
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

#ifndef FREEP
#define FREEP(p)        do { if (p) free(p); } while (0)
#endif

ISpellChecker::~ISpellChecker()
{
	if (m_bSuccessfulInit) {
		// only cleanup our mess if we were successfully initialized
		
		clearindex (m_pflagindex);
		clearindex (m_sflagindex);
	}

	FREEP(m_hashtbl);
	FREEP(m_hashstrings);
	FREEP(m_sflaglist);
	FREEP(m_chartypes);
	                                                	
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
	
	bool retVal = false;

	if (!g_iconv_is_valid(m_translate_in))
		return false;	
	else
		{
			/* convert to 8bit string and null terminate */
			size_t len_in, len_out, result;
			// the 8bit encodings use precomposed forms
			char *normalizedWord = g_utf8_normalize (utf8Word, length, G_NORMALIZE_NFC);
			char *In = normalizedWord;
			char *Out = szWord;
			
			len_in = strlen(In);
			len_out = sizeof( szWord ) - 1;
			result = g_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
			g_free(normalizedWord);
			if ((size_t)-1 == result)
				return false;
			*Out = '\0';
		}
	
	if (!strtoichar(iWord, szWord, sizeof(iWord), 0))
		{
			if (good(iWord, 0, 0, 1, 0) >= 1 ||
			    compoundgood(iWord, 1))
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
			
			size_t len_in, len_out, result;
			// the 8bit encodings use precomposed forms
			char *normalizedWord = g_utf8_normalize (utf8Word, length, G_NORMALIZE_NFC);
			char *In = normalizedWord;
			char *Out = word8;
			len_in = strlen(In);
			len_out = sizeof( word8 ) - 1;
			result = g_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
			g_free(normalizedWord);
			if ((size_t)-1 == result)
				return NULL;
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
				
				char *utf8Sugg = g_new0(char, INPUTWORDLEN + MAXAFFIXLEN + 1);
				
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
						len_out = INPUTWORDLEN + MAXAFFIXLEN;
						if ((size_t)-1 == g_iconv(m_translate_out, &In, &len_in, &Out, &len_out)) {
							*out_n_suggestions = c;
							return sugg_arr;
						}
						*(Out) = 0;
					}
				//test 
				sugg_arr[c] = "chenxiajian ispell";
			}
	}
	
	return sugg_arr;
}

<<<<<<< .mine
<<<<<<< .mine
<<<<<<< .mine

char **
ISpellChecker::hyphenate(const char * const utf8Word, size_t length,
						   size_t * out_n_suggestions)
{
	ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
	char word8[INPUTWORDLEN + MAXAFFIXLEN];
	int  c;

	*out_n_suggestions = 0;
	char **sugg_arr = NULL;
    ///not implement yet! chenxiajian///

	return sugg_arr;
}

=======

char *
ISpellChecker::hyphenate(const char * const utf8Word)
{
	ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
	char word8[INPUTWORDLEN + MAXAFFIXLEN];
	int  c;

	char **sugg_arr = NULL;
    ///not implement yet! chenxiajian///

	char*result=0;
	return result;	
}


>>>>>>> .theirs
=======

char *
ISpellChecker::hyphenate(const char * const utf8Word)
{
	ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
	char word8[INPUTWORDLEN + MAXAFFIXLEN];
	int  c;

	char **sugg_arr = NULL;
    ///not implement yet! chenxiajian///

	char*result=0;
	return result;	
}





















>>>>>>> .theirs
=======

char *
ISpellChecker::hyphenate(const char * const utf8Word)
{  //we must choose the right language tag
	const char* result=Hyphenator(RFC_3066::Language("en")).hyphenate(utf8Word).c_str();	
	char* temp=new char[strlen(result)];
	strcpy(temp,result);
	return temp;	
}
































































>>>>>>> .theirs
static GSList *
ispell_checker_get_dictionary_dirs (EnchantBroker * broker)
{
	GSList *dirs = NULL;

	{
		GSList *config_dirs, *iter;

		config_dirs = enchant_get_user_config_dirs ();
		
		for (iter = config_dirs; iter; iter = iter->next)
			{
				dirs = g_slist_append (dirs, g_build_filename ((const gchar *)iter->data, 
									       ENCHANT_ISPELL_HOME_DIR, NULL));
			}

		g_slist_foreach (config_dirs, (GFunc)g_free, NULL);
		g_slist_free (config_dirs);
	}

#if 0
	{
		const gchar* const * system_data_dirs = g_get_system_data_dirs ();
		const gchar* const * iter;

		for (iter = system_data_dirs; *iter; iter++)
			{
				dirs = g_slist_append (dirs, g_build_filename (*iter, "ispell", "dicts", NULL));
			}
	}
#endif

	/* until I work out how to link the modules against enchant in MacOSX - fjf
	 */
#ifndef XP_TARGET_COCOA
	char * ispell_prefix = NULL;

	/* Look for explicitly set registry values */
	ispell_prefix = enchant_get_registry_value ("Ispell", "Data_Dir");
	if (ispell_prefix)
		dirs = g_slist_append (dirs, ispell_prefix);

	/* Dynamically locate library and search for modules relative to it. */
	char * enchant_prefix = enchant_get_prefix_dir();
	if(enchant_prefix)
		{
			ispell_prefix = g_build_filename(enchant_prefix, "share", "enchant", "ispell", NULL);
			g_free(enchant_prefix);
			dirs = g_slist_append (dirs, ispell_prefix);
		}
#endif

#ifdef ENCHANT_ISPELL_DICT_DIR
	dirs = g_slist_append (dirs, g_strdup (ENCHANT_ISPELL_DICT_DIR));
#endif

	{
		GSList *config_dirs, *iter;

		config_dirs = enchant_get_dirs_from_param (broker, "enchant.ispell.dictionary.path");
		
		for (iter = config_dirs; iter; iter = iter->next)
			{
				dirs = g_slist_append (dirs, g_strdup ((const gchar *)iter->data));
			}

		g_slist_foreach (config_dirs, (GFunc)g_free, NULL);
		g_slist_free (config_dirs);
	}

	return dirs;
}

static void
s_buildHashNames (std::vector<std::string> & names, EnchantBroker * broker, const char * dict)
{
	names.clear ();

	GSList *dirs, *iter;

	dirs = ispell_checker_get_dictionary_dirs(broker);
	for (iter = dirs; iter; iter = iter->next)
		{
			char *tmp;

			tmp = g_build_filename ((const gchar *)iter->data, dict, NULL);
			names.push_back (tmp);
			g_free (tmp);
		}

	g_slist_foreach (dirs, (GFunc)g_free, NULL);
	g_slist_free (dirs);
}

char *
ISpellChecker::loadDictionary (const char * szdict)
{
	std::vector<std::string> dict_names;

	s_buildHashNames (dict_names, m_broker, szdict);

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
	
	const char * encoding = NULL;
	const char * szFile = NULL;
	
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
		{
			/* We still have to setup prefstringchar*/
			prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag
						      : static_cast<int *>(NULL));
			
			if (prefstringchar < 0)
				{
					char teststring[64];
					for(int n1 = 1; n1 <= 15; n1++)
						{
							sprintf(teststring, "latin%d", n1);
							prefstringchar = findfiletype(teststring, 1,				      
										      deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
							if (prefstringchar >= 0) 
								break;
						}
				}
			
			return; /* success */
		}
	
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
			} else
				return false;
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

<<<<<<< .mine
<<<<<<< .mine
<<<<<<< .mine
static char **
ispell_dict_hyphenate (EnchantDict * me, const char *const word,
					 size_t len, size_t * out_n_suggs)
{
	ISpellChecker * checker;

	checker = (ISpellChecker *) me->user_data;
	return checker->hyphenate (word, len, out_n_suggs);
}


=======
static char *
ispell_dict_hyphenate (EnchantDict * me, const char *const word)
{
	ISpellChecker * checker;

	checker = (ISpellChecker *) me->user_data;
	return checker->hyphenate (word);
}



>>>>>>> .theirs
=======
static char *
ispell_dict_hyphenate (EnchantDict * me, const char *const word)
{
	ISpellChecker * checker;

	checker = (ISpellChecker *) me->user_data;
	return checker->hyphenate (word);
}

















>>>>>>> .theirs
=======
static char *
ispell_dict_hyphenate (EnchantDict * me, const char *const word)
{
	ISpellChecker * checker;

	checker = (ISpellChecker *) me->user_data;
	return checker->hyphenate (word);
}













































>>>>>>> .theirs
static int
ispell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
	ISpellChecker * checker;
	
	checker = (ISpellChecker *) me->user_data;
	
	if (checker->checkWord(word, len))
		return 0;
	
	return 1;
}

static EnchantDict *
ispell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
	EnchantDict *dict;
	ISpellChecker * checker;
	
	checker = new ISpellChecker (me->owner);
	
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
	dict->hyphenate = ispell_dict_hyphenate;
	// don't implement session or personal
	
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

static int
_ispell_provider_dictionary_exists (EnchantBroker * broker, const char *const szFile)
{
	std::vector <std::string> names;

	s_buildHashNames (names, broker, szFile);
	for (size_t i = 0; i < names.size(); i++) {
		if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
			return 1;
	}

	return 0;
}

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *) 
	     init_enchant_provider (void);

static char **
ispell_provider_list_dictionaries (EnchantProvider * me,
				   size_t * out_n_dicts)
{
	size_t i, nb;
	char ** out_dicts = g_new0 (char *, size_ispell_map + 1);

	(void)me;

	nb = 0;
	for (i = 0; i < size_ispell_map; i++)
		if (_ispell_provider_dictionary_exists (me->owner, ispell_map[i].dict))
			out_dicts[nb++] = g_strdup (ispell_map[i].lang);

	*out_n_dicts = nb;
	if (nb == 0) {
		g_free (out_dicts);
		out_dicts = NULL;
	}

	return out_dicts;
}

static int
ispell_provider_dictionary_exists (struct str_enchant_provider * me,
				   const char *const tag)
{
	std::string shortened_dict (tag);
	size_t uscore_pos;
	if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1))
		shortened_dict = shortened_dict.substr(0, uscore_pos);
	
	for (size_t i = 0; i < size_ispell_map; i++)
		{
			const IspellMap * mapping = (const IspellMap *)(&(ispell_map[i]));
			if (!strcmp (tag, mapping->lang) || !strcmp (shortened_dict.c_str(), mapping->lang)) 
				return _ispell_provider_dictionary_exists(me->owner, mapping->dict);
		}
	
	return 0;
}

static void
ispell_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static void
ispell_provider_dispose (EnchantProvider * me)
{
	g_free (me);
}

static const char *
ispell_provider_identify (EnchantProvider * me)
{
	return "ispell";
}

static const char *
ispell_provider_describe (EnchantProvider * me)
{
	return "Ispell Provider";
}

EnchantProvider *
init_enchant_provider (void)
{
	EnchantProvider *provider;
	
	provider = g_new0 (EnchantProvider, 1);
	provider->dispose = ispell_provider_dispose;
	provider->request_dict = ispell_provider_request_dict;
	provider->dispose_dict = ispell_provider_dispose_dict;
	provider->dictionary_exists = ispell_provider_dictionary_exists;
	provider->identify = ispell_provider_identify;
	provider->describe = ispell_provider_describe;
	provider->list_dicts = ispell_provider_list_dictionaries;
	provider->free_string_list = ispell_provider_free_string_list;

	return provider;
}

} // extern C linkage
