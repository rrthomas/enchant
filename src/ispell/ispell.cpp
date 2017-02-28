/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "sp_spell.h"
#include "ispell.h"
#include "unused-parameter.h"

#define G_ICONV_INVALID (GIConv)-1

/***************************************************************************/

static const IspellMap ispell_map_array [] = {
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

static const size_t size_ispell_map = G_N_ELEMENTS(ispell_map_array);

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
     m_BC(nullptr),
     m_cd(nullptr),
     m_cl(nullptr),
     m_cm(nullptr),
     m_ho(nullptr),
     m_nd(nullptr),
     m_so(nullptr),
     m_se(nullptr),
     m_sg(0),
     m_ti(nullptr),
     m_te(nullptr),
     m_li(0),
     m_co(0),
     m_numhits(0),
     m_hashstrings(nullptr),
     m_hashheader({}),
     m_hashtbl(nullptr),
     m_hashsize(0),
     m_aflag(0),
     m_cflag(0),
     m_lflag(0),
     m_incfileflag(0),
     m_nodictflag(0),
     m_uerasechar(0),
     m_ukillchar(0),
     m_laststringch(0U),
     m_defdupchar(0),
     m_numpflags(0),
     m_numsflags(0),
     m_pflaglist(nullptr),
     m_sflaglist(nullptr),
     m_chartypes(nullptr),
     m_infile(nullptr),
     m_outfile(nullptr),
     m_askfilename(nullptr),
     m_changes(0),
     m_readonly(0),
     m_quit(0),
     m_pcount(0),
     m_maxposslen(0),
     m_easypossibilities(0),
     m_Trynum(0),
     m_translate_in(G_ICONV_INVALID),
     m_translate_out(G_ICONV_INVALID)
{
	memset(m_sflagindex,0,sizeof(m_sflagindex));
	memset(m_pflagindex,0,sizeof(m_pflagindex));
}

ISpellChecker::~ISpellChecker()
{
	if (m_bSuccessfulInit) {
		// only cleanup our mess if we were successfully initialized
		
		clearindex (m_pflagindex);
		clearindex (m_sflagindex);
	}

	free(m_hashtbl);
	free(m_hashstrings);
	free(m_sflaglist);
	free(m_chartypes);
	                                                	
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
			if (static_cast<size_t>(-1) == result)
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
			if (static_cast<size_t>(-1) == result)
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
						char *Out = utf8Sugg;
						
						len_in = l;
						len_out = INPUTWORDLEN + MAXAFFIXLEN;
						if (static_cast<size_t>(-1) == g_iconv(m_translate_out, &In, &len_in, &Out, &len_out)) {
							*out_n_suggestions = c;
							return sugg_arr;
						}
						*(Out) = 0;
					}
				
				sugg_arr[c] = utf8Sugg;
			}
	}
	
	return sugg_arr;
}

size_t
ISpellChecker::ispell_map_size (void)
{
	return size_ispell_map;
}

const IspellMap *
ISpellChecker::ispell_map (void)
{
	return &ispell_map_array[0];
}

char *
ISpellChecker::loadDictionary (const char * szdict, std::vector<std::string> dict_names)
{
	for (size_t i = 0; i < dict_names.size(); i++)
		{
			if (linit(const_cast<char*>(dict_names[i].c_str())) >= 0)
				return g_strdup (dict_names[i].c_str());
		}

	return NULL;
}

/*!
 * Load ispell dictionary hash file.
 *
 * \param szFile -   The name of the dictionary file
 * \param encoding - The encoding of the dictionary file
 * \return Whether dictionary was successfully loaded
 */
bool
ISpellChecker::loadDictionaryWithEncoding (const char * szFile, const char * encoding, std::vector<std::string> dict_names)
{
	char *szDict = NULL;
	char *hashname = NULL;
	
	for (size_t i = 0; i < dict_names.size(); i++)
		{
			if (linit(const_cast<char*>(dict_names[i].c_str())) >= 0)
				szDict = g_strdup (dict_names[i].c_str());
		}

	if (szDict == NULL)
		return false;

	alloc_ispell_struct();
	
	if (!(hashname = loadDictionary(szDict, dict_names)))
		return false;
	
	// one of the two above calls succeeded
	setDictionaryEncoding (encoding);
	g_free (hashname);

	return true;
}

void
ISpellChecker::setDictionaryEncoding( const char * encoding )
{
	/* Get Hash encoding from XML file. This should always work! */
	try_autodetect_charset(encoding);

	if (g_iconv_is_valid(m_translate_in) && g_iconv_is_valid(m_translate_out))
		{
			/* We still have to setup prefstringchar*/
			prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag
						      : nullptr);
			
			if (prefstringchar < 0)
				{
					char teststring[64];
					for(int n1 = 1; n1 <= 15; n1++)
						{
							sprintf(teststring, "latin%d", n1);
							prefstringchar = findfiletype(teststring, 1,				      
										      deftflag < 0 ? &deftflag : nullptr);
							if (prefstringchar >= 0) 
								break;
						}
				}
			
			return; /* success */
		}
	
	/* Test for UTF-8 first */
	prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : nullptr);
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
			for(unsigned n1 = 1; n1 <= 15; n1++)
				{
					char * teststring = g_strdup_printf("latin%u", n1);
					prefstringchar = findfiletype(teststring, 1,
								      deftflag < 0 ? &deftflag : nullptr);
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

void
ISpellChecker::finishInit()
{
        m_bSuccessfulInit = true;

        if (prefstringchar < 0)
                m_defdupchar = 0;
        else
                m_defdupchar = prefstringchar;
}
