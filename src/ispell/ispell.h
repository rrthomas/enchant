/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef ISPELL_CHECKER_H
#define ISPELL_CHECKER_H

#include <string>
#include <vector>

#include <glib.h> // give glib a chance to override MAXPATHLEN first before it is set in ispell.h
#include "enchant.h"


/******* START OF OLD ispell.h ******/

#include <sys/types.h>

/*
 * Copyright 1992, 1993, Geoff Kuenning, Granada Hills, CA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Geoff Kuenning and
 *      other unpaid contributors.
 * 5. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Forked from Revision 1.68 of upstream ispell.h. */

#include <stdio.h>
/*  #include "ut_types.h" */

#include "ispell_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* c++ */

/* largest amount that a word might be extended by adding affixes */
#ifndef MAXAFFIXLEN
#define MAXAFFIXLEN 20
#endif

/*
** Number of mask bits (affix flags) supported.  Must be 32, 64, 128, or
** 256.  If MASKBITS is 32 or 64, there are really only 26 or 58 flags
** available, respectively.  If it is 32, the flags are named with the
** 26 English uppercase letters;  lowercase will be converted to uppercase.
** If MASKBITS is 64, the 58 flags are named 'A' through 'z' in ASCII
** order, including the 6 special characters from 'Z' to 'a': "[\]^_`".
** If MASKBITS is 128 or 256, all the 7-bit or 8-bit characters,
** respectively, are theoretically available, though a few (newline, slash,
** null byte) are pretty hard to actually use successfully.
**
** Note that a number of non-English affix files depend on having a
** larger value for MASKBITS.  See the affix files for more
** information.
*/

#ifndef MASKBITS
#define MASKBITS	64
#endif

extern int		gnMaskBits;

/*
** C type to use for masks.  This should be a type that the processor
** accesses efficiently.
**
** MASKTYPE_WIDTH must correctly reflect the number of bits in a
** MASKTYPE.  Unfortunately, it is also required to be a constant at
** preprocessor time, which means you can't use the sizeof operator to
** define it.
**
** Note that MASKTYPE *must* match MASKTYPE_WIDTH or you may get
** division-by-zero errors! 
*/
#ifndef MASKTYPE
#define MASKTYPE	long
#endif
#ifndef MASKTYPE_WIDTH
#define MASKTYPE_WIDTH	32
#endif

  /* program: this should be coded now in init */

#if MASKBITS < MASKTYPE_WIDTH
#undef MASKBITS
#define MASKBITS	MASKTYPE_WIDTH
#endif /* MASKBITS < MASKTYPE_WIDTH */

/*
** Maximum hash table fullness percentage.  Larger numbers trade space
** for time.
**/
#ifndef MAXPCT
#define MAXPCT	70		/* Expand table when 70% full */
#endif

/*
** Maximum number of "string" characters that can be defined in a
** language (affix) file.  Don't forget that an upper/lower string
** character counts as two!
*/
#ifndef MAXSTRINGCHARS
#define MAXSTRINGCHARS 512
#endif /* MAXSTRINGCHARS */

/*
** Maximum length of a "string" character.  The default is appropriate for
** nroff-style characters starting with a backslash.
*/
#ifndef MAXSTRINGCHARLEN
#define MAXSTRINGCHARLEN 10
#endif /* MAXSTRINGCHARLEN */

/*
** Maximum number of "hits" expected on a word.  This is basically the
** number of different ways different affixes can produce the same word.
** For example, with "english.aff", "brothers" can be produced 3 ways:
** "brothers," "brother+s", or "broth+ers".  If this is too low, no major
** harm will be done, but ispell may occasionally forget a capitalization.
*/
#ifndef MAX_HITS
#define MAX_HITS	10
#endif

/*
** Maximum number of capitalization variations expected in any word.
** Besides the obvious all-lower, all-upper, and capitalized versions,
** this includes followcase variants.  If this is too low, no real
** harm will be done, but ispell may occasionally fail to suggest a
** correct capitalization.
*/
#ifndef MAX_CAPS
#define MAX_CAPS	10
#endif /* MAX_CAPS */

/* buffer size to use for file names if not in sys/param.h */
#ifndef MAXPATHLEN
#define MAXPATHLEN 512
#endif

/*
** Maximum language-table search size.  Smaller numbers make ispell
** run faster, at the expense of more memory (the lowest reasonable value
** is 2).  If a given character appears in a significant position in
** more than MAXSEARCH suffixes, it will be given its own index table.
** If you change this, define INDEXDUMP in lookup.c to be sure your
** index table looks reasonable.
*/
#ifndef MAXSEARCH
#define MAXSEARCH 4
#endif

#if defined(__STDC__) || defined(__cplusplus)
#define P(x)	x
 #ifndef VOID
   #define VOID	void
 #endif
#else /* __STDC__ */
#define P(x)	()
 #ifndef VOID
   #define VOID	char
 #endif
#define const
#endif /* __STDC__ */

#define SET_SIZE	256

#define MASKSIZE	(gnMaskBits / MASKTYPE_WIDTH)

#ifdef lint
extern int	TSTMASKBIT P ((MASKTYPE * mask, int bit));
#else /* lint */
/* The following is really testing for MASKSIZE <= 1, but cpp can't do that */
#define TSTMASKBIT(mask, bit) \
		    ((mask)[(bit) / MASKTYPE_WIDTH] & \
		      ((MASKTYPE) 1 << ((bit) & (MASKTYPE_WIDTH - 1))))
#endif /* lint */

#if MASKBITS > 64
#define FULLMASKSET
#endif

#if MASKBITS <= 32
	#define FLAGBASE	((MASKTYPE_WIDTH) - 6)
#else
	# if MASKBITS <= 64
		#define FLAGBASE	((MASKTYPE_WIDTH) - 6)
	# else
		#define FLAGBASE	0
	# endif
#endif

typedef unsigned short	ichar_t;	/* Internal character */

/*
 * Structure used to record data about successful lookups; these values
 * are used in the ins_root_cap routine to produce correct capitalizations.
 */
struct success
{
    struct dent *		dictent;	/* Header of dict entry chain for wd */
    struct flagent *	prefix;		/* Prefix flag used, or NULL */
    struct flagent *	suffix;		/* Suffix flag used, or NULL */
};

ichar_t* icharcpy (ichar_t* out, ichar_t* in);
int icharlen (ichar_t* in);
int icharcmp (ichar_t* s1, ichar_t* s2);
int icharncmp (ichar_t* s1, ichar_t* s2, int n);

struct dent
{
    struct dent *	next;
    char *			word;
    MASKTYPE		mask[2];
#ifdef FULLMASKSET
    char			flags;
#endif
};

/*
** Flags in the directory entry.  If FULLMASKSET is undefined, these are
** stored in the highest bits of the last longword of the mask field.  If
** FULLMASKSET is defined, they are stored in the extra "flags" field.
**
** If a word has only one capitalization form, and that form is not
** FOLLOWCASE, it will have exactly one entry in the dictionary.  The
** legal capitalizations will be indicated by the 2-bit capitalization
** field, as follows:
**
**	ALLCAPS		The word must appear in all capitals.
**	CAPITALIZED	The word must be capitalized (e.g., London).
**			It will also be accepted in all capitals.
**	ANYCASE		The word may appear in lowercase, capitalized,
**			or all-capitals.
**
** Regardless of the capitalization flags, the "word" field of the entry
** will point to an all-uppercase copy of the word.  This is to simplify
** the large portion of the code that doesn't care about capitalization.
** Ispell will generate the correct version when needed.
**
** If a word has more than one capitalization, there will be multiple
** entries for it, linked together by the "next" field.  The initial
** entry for such words will be a dummy entry, primarily for use by code
** that ignores capitalization.  The "word" field of this entry will
** again point to an all-uppercase copy of the word.  The "mask" field
** will contain the logical OR of the mask fields of all variants.
** A header entry is indicated by a capitalization type of ALLCAPS,
** with the MOREVARIANTS bit set.
**
** The following entries will define the individual variants.  Each
** entry except the last has the MOREVARIANTS flag set, and each
** contains one of the following capitalization options:
**
**	ALLCAPS		The word must appear in all capitals.
**	CAPITALIZED	The word must be capitalized (e.g., London).
**			It will also be accepted in all capitals.
**	FOLLOWCASE	The word must be capitalized exactly like the
**			sample in the entry.  Prefix (suffix) characters
**			must be rendered in the case of the first (last)
**			"alphabetic" character.  It will also be accepted
**			in all capitals.  ("Alphabetic" means "mentioned
**			in a 'casechars' statement".)
**	ANYCASE		The word may appear in lowercase, capitalized,
**			or all-capitals.
**
** The "mask" field for the entry contains only the affix flag bits that
** are legal for that capitalization.  The "word" field will be null
** except for FOLLOWCASE entries, where it will point to the
** correctly-capitalized spelling of the root word.
**
** It is worth discussing why the ALLCAPS option is used in
** the header entry.  The header entry accepts an all-capitals
** version of the root plus every affix (this is always legal, since
** words get capitalized in headers and so forth).  Further, all of
** the following variant entries will reject any all-capitals form
** that is illegal due to an affix.
**
** Finally, note that variations in the KEEP flag can cause a multiple-variant
** entry as well.  For example, if the personal dictionary contains "ALPHA",
** (KEEP flag set) and the user adds "alpha" with the KEEP flag clear, a
** multiple-variant entry will be created so that "alpha" will be accepted
** but only "ALPHA" will actually be kept.
*/
#ifdef FULLMASKSET
#define flagfield	flags
#else
#define flagfield	mask[1]
#endif
#define USED		((MASKTYPE) 1 << (FLAGBASE + 0))
#define KEEP		((MASKTYPE) 1 << (FLAGBASE + 1))
#define ANYCASE		((MASKTYPE) 0 << (FLAGBASE + 2))
#define ALLCAPS		((MASKTYPE) 1 << (FLAGBASE + 2))
#define CAPITALIZED	((MASKTYPE) 2 << (FLAGBASE + 2))
#define FOLLOWCASE	((MASKTYPE) 3 << (FLAGBASE + 2))
#define CAPTYPEMASK	((MASKTYPE) 3 << (FLAGBASE + 2))
#define MOREVARIANTS	((MASKTYPE) 1 << (FLAGBASE + 4))
#define ALLFLAGS	(USED | KEEP | CAPTYPEMASK | MOREVARIANTS)
#define captype(x)	((x) & CAPTYPEMASK)

/*
 * Language tables used to encode prefix and suffix information.
 */
struct flagent
{
    ichar_t *		strip;		/* String to strip off */
    ichar_t *		affix;		/* Affix to append */
    short		flagbit;		/* Flag bit this ent matches */
    short		stripl;			/* Length of strip */
    short		affl;			/* Length of affix */
    short		numconds;		/* Number of char conditions */
    short		flagflags;		/* Modifiers on this flag */
    char		conds[SET_SIZE + MAXSTRINGCHARS]; /* Adj. char conds */
};

/*
 * Bits in flagflags
 */
#define FF_CROSSPRODUCT	(1 << 0)		/* Affix does cross-products */
#define FF_COMPOUNDONLY	(1 << 1)		/* Afx works in compounds */

union ptr_union					/* Aid for building flg ptrs */
{
    struct flagptr *	fp;			/* Pointer to more indexing */
    struct flagent *	ent;		/* First of a list of ents */
};

struct flagptr
{
    union ptr_union	pu;			/* Ent list or more indexes */
    int			numents;		/* If zero, pu.fp is valid */
};

/*
 * Description of a single string character type.
 */
struct strchartype
{
    char *		name;			/* Name of the type */
    char *		deformatter;	/* Deformatter to use */
    char *		suffixes;		/* File suffixes, null seps */
};

/*
 * Header placed at the beginning of the hash file.
 */
struct hashheader
{
    unsigned short magic;    	    	    	/* Magic number for ID */
    unsigned short compileoptions;				/* How we were compiled */
    short maxstringchars;						/* Max # strchrs we support */
    short maxstringcharlen;						/* Max strchr len supported */
    short compoundmin;							/* Min lth of compound parts */
    short compoundbit;							/* Flag 4 compounding roots */
    int stringsize;								/* Size of string table */
    int lstringsize;							/* Size of lang. str tbl */
    int tblsize;								/* No. entries in hash tbl */
    int stblsize;								/* No. entries in sfx tbl */
    int ptblsize;								/* No. entries in pfx tbl */
    int sortval;								/* Largest sort ID assigned */
    int nstrchars;								/* No. strchars defined */
    int nstrchartype;							/* No. strchar types */
    int strtypestart;							/* Start of strtype table */
    char nrchars[5];							/* Nroff special characters */
    char texchars[13];							/* TeX special characters */
    char compoundflag;							/* Compund-word handling */
    char defhardflag;							/* Default tryveryhard flag */
    char flagmarker;							/* "Start-of-flags" char */
    unsigned short sortorder[SET_SIZE + MAXSTRINGCHARS]; /* Sort ordering */
    ichar_t lowerconv[SET_SIZE + MAXSTRINGCHARS]; /* Lower-conversion table */
    ichar_t upperconv[SET_SIZE + MAXSTRINGCHARS]; /* Upper-conversion table */
    char wordchars[SET_SIZE + MAXSTRINGCHARS]; /* NZ for chars found in wrds */
    char upperchars[SET_SIZE + MAXSTRINGCHARS]; /* NZ for uppercase chars */
    char lowerchars[SET_SIZE + MAXSTRINGCHARS]; /* NZ for lowercase chars */
    char boundarychars[SET_SIZE + MAXSTRINGCHARS]; /* NZ for boundary chars */
    char stringstarts[SET_SIZE];		/* NZ if char can start str */
    char stringchars[MAXSTRINGCHARS][MAXSTRINGCHARLEN + 1]; /* String chars */
    unsigned int stringdups[MAXSTRINGCHARS];	/* No. of "base" char */
    int dupnos[MAXSTRINGCHARS];			/* Dup char ID # */
    unsigned short magic2;			/* Second magic for dbl chk */
};

/* hash table magic number */
#define MAGIC			0x9602

/* compile options, put in the hash header for consistency checking */
#define MAGIC8BIT		0x00
#define MAGICCAPITALIZATION	0x02

#if MASKBITS <= 32
# define MAGICMASKSET		0x00
#else
# if MASKBITS <= 64
#  define MAGICMASKSET		0x04
# else
#  if MASKBITS <= 128
#   define MAGICMASKSET		0x08
#  else
#   define MAGICMASKSET		0x0C
#  endif
# endif
#endif

#define COMPILEOPTIONS	(MAGIC8BIT | MAGICCAPITALIZATION | MAGICMASKSET)

/*
** Offsets into the nroff special-character array
*/
#define NRLEFTPAREN		hashheader.nrchars[0]
#define NRRIGHTPAREN	hashheader.nrchars[1]
#define NRDOT			hashheader.nrchars[2]
#define NRBACKSLASH		hashheader.nrchars[3]
#define NRSTAR			hashheader.nrchars[4]

/*
** Offsets into the TeX special-character array
*/
#define TEXLEFTPAREN	hashheader.texchars[0]
#define TEXRIGHTPAREN	hashheader.texchars[1]
#define TEXLEFTSQUARE	hashheader.texchars[2]
#define TEXRIGHTSQUARE	hashheader.texchars[3]
#define TEXLEFTCURLY	hashheader.texchars[4]
#define TEXRIGHTCURLY	hashheader.texchars[5]
#define TEXLEFTANGLE	hashheader.texchars[6]
#define TEXRIGHTANGLE	hashheader.texchars[7]
#define TEXBACKSLASH	hashheader.texchars[8]
#define TEXDOLLAR		hashheader.texchars[9]
#define TEXSTAR			hashheader.texchars[10]
#define TEXDOT			hashheader.texchars[11]
#define TEXPERCENT		hashheader.texchars[12]

/*
** Values for compoundflag
*/
#define COMPOUND_NEVER		0	/* Compound words are never good */
#define COMPOUND_ANYTIME	1	/* Accept run-together words */
#define COMPOUND_CONTROLLED	2	/* Compounds controlled by afx flags */
/*
** These macros are similar to the ones above, but they take into account
** the possibility of string characters.  Note well that they take a POINTER,
** not a character.
**
** The "l_" versions set "len" to the length of the string character as a
** handy side effect.  (Note that the global "laststringch" is also set,
** and sometimes used, by these macros.)
**
** The "l1_" versions go one step further and guarantee that the "len"
** field is valid for *all* characters, being set to 1 even if the macro
** returns false.  This macro is a great example of how NOT to write
** readable C.
*/
/*TF NOTE: This is actually defined in code (makedent) now */
#if 0 
#define isstringch(ptr, canon)	(isstringstart (*(ptr)) \
				  &&  stringcharlen ((ptr), (canon)) > 0)
#define l_isstringch(ptr, len, canon)	\
				(isstringstart (*(ptr)) \
				  &&  (len = stringcharlen ((ptr), (canon))) \
				    > 0)
#define l1_isstringch(ptr, len, canon)	\
				(len = 1, \
				  isstringstart ((unsigned char)(*(ptr))) \
				    &&  ((len = \
					  stringcharlen ((ptr), (canon))) \
					> 0 \
				      ? 1 : (len = 1, 0)))
#endif

/*
 * Sizes of buffers returned by ichartosstr/strtosichar.
 */
#define ICHARTOSSTR_SIZE (INPUTWORDLEN + 4 * MAXAFFIXLEN + 4)
#define STRTOSICHAR_SIZE ((INPUTWORDLEN + 4 * MAXAFFIXLEN + 4) \
			  * sizeof (ichar_t))
/* TF CHANGE: We should fill this as a structure
              and then use it throughout.
*/

/*
 * Initialized variables.  These are generated using macros so that they
 * may be consistently declared in all programs.  Numerous examples of
 * usage are given below.
 */
#ifdef MAIN
#define INIT(decl, init)	decl = init
#else
#define INIT(decl, init)	extern decl
#endif

#ifdef MINIMENU
INIT (int minimenusize, 2);		/* MUST be either 2 or zero */
#else /* MINIMENU */
INIT (int minimenusize, 0);		/* MUST be either 2 or zero */
#endif /* MINIMENU */

INIT (int eflag, 0);			/* NZ for expand mode */
INIT (int dumpflag, 0);			/* NZ to do dump mode */
INIT (int fflag, 0);			/* NZ if -f specified */
#ifndef USG
INIT (int sflag, 0);			/* NZ to stop self after EOF */
#endif
INIT (int vflag, 0);			/* NZ to display characters as M-xxx */
INIT (int xflag, DEFNOBACKUPFLAG);	/* NZ to suppress backups */
INIT (int deftflag, -1);		/* NZ for TeX mode by default */
INIT (int tflag, DEFTEXFLAG);		/* NZ for TeX mode in current file */
INIT (int prefstringchar, -1);		/* Preferred string character type */

INIT (int terse, 0);			/* NZ for "terse" mode */

INIT (char tempfile[MAXPATHLEN], "");	/* Name of file we're spelling into */

INIT (int minword, MINWORD);		/* Longest always-legal word */
INIT (int sortit, 1);			/* Sort suggestions alphabetically */
INIT (int compoundflag, -1);		/* How to treat compounds: see above */
INIT (int tryhardflag, -1);		/* Always call tryveryhard */

INIT (char * currentfile, NULL);	/* Name of current input file */

/* Odd numbers for math mode in LaTeX; even for LR or paragraph mode */
INIT (int math_mode, 0);
/* P -- paragraph or LR mode
 * b -- parsing a \begin statement
 * e -- parsing an \end statement
 * r -- parsing a \ref type of argument.
 * m -- looking for a \begin{minipage} argument.
 */
INIT (char LaTeX_Mode, 'P');

#ifdef __cplusplus
}
#endif /* c++ */

/******* END OF OLD ispell.h ******/


typedef struct str_ispell_map
{
	const char * lang;
	const char * dict;
	const char * enc;
} IspellMap;

class ISpellChecker
{
public:
	ISpellChecker();
	~ISpellChecker();

	bool checkWord(const char * const word, size_t len);
	char ** suggestWord(const char * const word, size_t len, size_t * out_n_suggs);

        void finishInit();
	bool requestDictionary (const char * szLang);
	bool loadDictionaryWithEncoding(const char * szFile, const char * encoding, std::vector<std::string> dict_names);

	size_t ispell_map_size (void);
	const IspellMap *ispell_map (void);

private:
	EnchantBroker* m_broker;

	ISpellChecker(const ISpellChecker&);	// no impl
	void operator=(const ISpellChecker&);	// no impl

	char * loadDictionary (const char * szLang, std::vector<std::string> dict_names );
	void   setDictionaryEncoding ( const char * enc );

	//
	// The member functions after this point were formerly global functions
	//  passed a context structure pointer...
	//

	void try_autodetect_charset(const char * inEncoding);

	//
	// From ispell correct.c
	//

	int		casecmp P ((char * a, char * b, int canonical));
	void		makepossibilities P ((ichar_t * word));
	int	insert P ((ichar_t * word));
	void	wrongcapital P ((ichar_t * word));
	void	wrongletter P ((ichar_t * word));
	void	extraletter P ((ichar_t * word));
	void	missingletter P ((ichar_t * word));
	void	missingspace P ((ichar_t * word));
	int		compoundgood P ((ichar_t * word, int pfxopts));
	void	transposedletter P ((ichar_t * word));
	int	ins_cap P ((ichar_t * word, ichar_t * pattern));
	int	save_cap P ((ichar_t * word, ichar_t * pattern,
			  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN]));
	int		ins_root_cap P ((ichar_t * word, ichar_t * pattern,
			  int prestrip, int preadd, int sufstrip, int sufadd,
			  struct dent * firstdent, struct flagent * pfxent,
			  struct flagent * sufent));
	void	save_root_cap P ((ichar_t * word, ichar_t * pattern,
			  int prestrip, int preadd, int sufstrip, int sufadd,
			  struct dent * firstdent, struct flagent * pfxent,
			  struct flagent * sufent,
			  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN],
			  int * nsaved));

	//
	// From ispell good.c
	//

	int good (ichar_t* w, int ignoreflagbits, int allhits, int pfxopts, int sfxopts);
	void chk_aff (ichar_t* word, ichar_t* ucword, int len, int ignoreflagbits, int allhits, int pfxopts, int sfxopts);
	int linit(char*);
	struct dent * ispell_lookup (ichar_t* s, int dotree);
	int strtoichar (ichar_t* out, char* in, int outlen, int canonical);
	int ichartostr (char* out, ichar_t* in, int outlen, int canonical);
	char * ichartosstr (ichar_t* in, int canonical);
	int	findfiletype (const char * name, int searchnames, int * deformatter);
	long whatcap (ichar_t* word);

	/*
		HACK: macros replaced with function implementations 
		so we could do a side-effect-free check for unicode
		characters which aren't in hashheader
	*/
	char myupper(ichar_t c);
	char mylower(ichar_t c);
	int myspace(ichar_t c);
	char iswordch(ichar_t c);
	char isboundarych(ichar_t c);
	char isstringstart(ichar_t c);
	ichar_t mytolower(ichar_t c);
	ichar_t mytoupper(ichar_t c);

	int cap_ok (ichar_t* word, struct success* hit, int len);
	int hash (ichar_t* s, int hashtblsize);

	//
	// From ispell lookup.c
	//

	void	clearindex P ((struct flagptr * indexp));
	void     initckch P ((char *));

	void alloc_ispell_struct();
	void free_ispell_struct();

	//
	// From ispell makedent.c
	//

	int		addvheader P ((struct dent * ent));
	void		upcase P ((ichar_t * string));
	void		lowcase P ((ichar_t * string));
	void		chupcase P ((char * s));

	int		stringcharlen P ((char * bufp, int canonical));
	ichar_t *	strtosichar P ((char * in, int canonical));
	char *		printichar P ((int in));

	//
	// From ispell tgood.c
	//

	void	pfx_list_chk P ((ichar_t * word, ichar_t * ucword,
			  int len, int optflags, int sfxopts, struct flagptr * ind,
			  int ignoreflagbits, int allhits));
	void	chk_suf P ((ichar_t * word, ichar_t * ucword, int len,
			  int optflags, struct flagent * pfxent, int ignoreflagbits,
			  int allhits));
	void	suf_list_chk P ((ichar_t * word, ichar_t * ucword, int len,
			  struct flagptr * ind, int optflags, struct flagent * pfxent,
			  int ignoreflagbits, int allhits));
	int		expand_pre P ((char * croot, ichar_t * rootword,
			  MASKTYPE mask[], int option, char * extra));
	int	pr_pre_expansion P ((char * croot, ichar_t * rootword,
			  struct flagent * flent, MASKTYPE mask[], int option,
			  char * extra));
	int		expand_suf P ((char * croot, ichar_t * rootword,
			  MASKTYPE mask[], int optflags, int option, char * extra));
	int	pr_suf_expansion P ((char * croot, ichar_t * rootword,
			  struct flagent * flent, int option, char * extra));
	void	forcelc P ((ichar_t * dst, int len));

	/* this is used for converting form unsigned short to UCS-4 */

	int deftflag;              /* NZ for TeX mode by default */
	int prefstringchar;        /* Preferred string character type */
	bool m_bSuccessfulInit;

	//
	// The members after this point were formerly global variables
	//  in the original ispell code
	//

	char *	m_BC;	/* backspace if not ^H */
	char *	m_cd;	/* clear to end of display */
	char *	m_cl;	/* clear display */
	char *	m_cm;	/* cursor movement */
	char *	m_ho;	/* home */
	char *	m_nd;	/* non-destructive space */
	char *	m_so;	/* standout */
	char *	m_se;	/* standout end */
	int	m_sg;	/* space taken by so/se */
	char *	m_ti;	/* terminal initialization sequence */
	char *	m_te;	/* terminal termination sequence */
	int	m_li;	/* lines */
	int	m_co;	/* columns */

	char	m_ctoken[INPUTWORDLEN + MAXAFFIXLEN]; /* Current token as char */
	ichar_t	m_itoken[INPUTWORDLEN + MAXAFFIXLEN]; /* Ctoken as ichar_t str */

	int	m_numhits;	/* number of hits in dictionary lookups */
	struct success
			m_hits[MAX_HITS]; /* table of hits gotten in lookup */

	char *	m_hashstrings;	/* Strings in hash table */
	struct hashheader
			m_hashheader;	/* Header of hash table */
	struct dent *
			m_hashtbl;	/* Main hash table, for dictionary */
	int	m_hashsize;	/* Size of main hash table */

	char	m_hashname[MAXPATHLEN]; /* Name of hash table file */

	int	m_aflag;		/* NZ if -a or -A option specified */
	int	m_cflag;		/* NZ if -c (crunch) option */
	int	m_lflag;		/* NZ if -l (list) option */
	int	m_incfileflag;	/* whether xgets() acts exactly like gets() */
	int	m_nodictflag;	/* NZ if dictionary not needed */

	int	m_uerasechar;	/* User's erase character, from stty */
	int	m_ukillchar;	/* User's kill character */

	unsigned int m_laststringch; /* Number of last string character */
	int	m_defdupchar;	/* Default duplicate string type */

	int	m_numpflags;		/* Number of prefix flags in table */
	int	m_numsflags;		/* Number of suffix flags in table */
	struct flagptr m_pflagindex[SET_SIZE + MAXSTRINGCHARS];
						/* Fast index to pflaglist */
	struct flagent *	m_pflaglist;	/* Prefix flag control list */
	struct flagptr m_sflagindex[SET_SIZE + MAXSTRINGCHARS];
						/* Fast index to sflaglist */
	struct flagent *	m_sflaglist;	/* Suffix flag control list */

	struct strchartype *		/* String character type collection */
			m_chartypes;

	FILE *	m_infile;			/* File being corrected */
	FILE *	m_outfile;		/* Corrected copy of infile */

	char *	m_askfilename;		/* File specified in -f option */

	int	m_changes;		/* NZ if changes made to cur. file */
	int	m_readonly;		/* NZ if current file is readonly */
	int	m_quit;			/* NZ if we're done with this file */

#define MAXPOSSIBLE	100	/* Max no. of possibilities to generate */

	char	m_possibilities[MAXPOSSIBLE][INPUTWORDLEN + MAXAFFIXLEN];
					/* Table of possible corrections */
	int	m_pcount;		/* Count of possibilities generated */
	int	m_maxposslen;	/* Length of longest possibility */
	int	m_easypossibilities; /* Number of "easy" corrections found */
					/* ..(defined as those using legal affixes) */

	/*
	 * The following array contains a list of characters that should be tried
	 * in "missingletter."  Note that lowercase characters are omitted.
	 */
	int	m_Trynum;		/* Size of "Try" array */
	ichar_t	m_Try[SET_SIZE + MAXSTRINGCHARS];

	GIConv  m_translate_in; /* Selected translation from/to Unicode */
	GIConv  m_translate_out;
};

#endif /* ISPELL_CHECKER_H */
