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
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#ifndef ISPELL_H
#define ISPELL_H

#include <sys/types.h>

/*
 * $Id$
 */

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

/*
 * $Log$
 * Revision 1.4  2003/08/14 17:51:27  dom
 * update license - exception clause should be Lesser GPL
 *
 * Revision 1.3  2003/07/28 20:40:26  dom
 * fix up the license clause, further win32-registry proof some directory getting functions
 *
 * Revision 1.2  2003/07/16 22:52:40  dom
 * LGPL + exception license
 *
 * Revision 1.1  2003/07/15 01:15:06  dom
 * ispell enchant backend
 *
 * Revision 1.10  2003/01/24 05:52:33  hippietrail
 *
 * Refactored ispell code. Old ispell global variables had been put into
 * an allocated structure, a pointer to which was passed to many functions.
 * I have now made all such functions and variables private members of the
 * ISpellChecker class. It was C OO, now it's C++ OO.
 *
 * I've fixed the makefiles and tested compilation but am unable to test
 * operation. Please back out my changes if they cause problems which
 * are not obvious or easy to fix.
 *
 * Revision 1.9  2002/09/19 05:31:15  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.8  2002/09/17 03:03:29  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.7  2002/03/22 14:31:57  dom
 * fix mg's compile problem
 *
 * Revision 1.6  2002/03/05 16:55:52  dom
 * compound word support, tested against swedish
 *
 * Revision 1.5  2001/08/10 18:32:40  dom
 * Spelling and iconv updates. god, i hate iconv
 *
 * Revision 1.4  2001/06/26 16:33:27  dom
 * 128 StringChars and some other stuff
 *
 * Revision 1.3  2001/05/12 16:05:42  thomasf
 * Big pseudo changes to ispell to make it pass around a structure rather
 * than rely on all sorts of gloabals willy nilly here and there.  Also
 * fixed our spelling class to work with accepting suggestions once more.
 * This code is dirty, gross and ugly (not to mention still not supporting
 * multiple hash sized just yet) but it works on my machine and will no
 * doubt break other machines.
 *
 * Revision 1.2  2001/04/18 00:59:36  thomasf
 * Removed the duplicate declarations of variables that was causing build
 * to bail.  This new ispell stuff is a total mess.
 *
 * Revision 1.1  2001/04/15 16:01:24  tomas_f
 * moving to spell/xp
 *
 * Revision 1.13  2001/04/13 12:33:12  tamlin
 * ispell can now be used from C++
 *
 * Revision 1.12  2001/03/25 01:30:02  tomb
 * 1. Fixed ispell #define problems on Win32
 * 2. Changed the way that togglable toolbars are tracked so that Full
 * Screen mode works right on Windows
 * 3. Fixed SET_GATHER macro in ap_Win32Dialog_Options.h
 * 4. Fixed Toggle Case dialog to default to Sentence Case when loaded
 * 5. Added #define for Auto Save checkbox (though I haven't updated the
 * Prefs dialog yet)
 *
 * Revision 1.11  2001/03/24 23:28:41  dom
 * Make C++ aware and watch out for VOID on Win32
 *
 * Revision 1.10  1999/12/21 18:46:29  sterwill
 * ispell patch for non-English dictionaries by Henrik Berg <henrik@lansen.se>
 *
 * Revision 1.9  1999/10/20 03:19:35  paul
 * Hacked ispell code to ignore any characters that don't fit in the lookup tables loaded from the dictionary.  It ain't pretty, but at least we don't crash there any more.
 *
 * Revision 1.8  1999/09/29 23:33:32  justin
 * Updates to the underlying ispell-based code to support suggested corrections.
 *
 * Revision 1.7  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.6  1999/01/07 05:14:22  sterwill
 * So it builds on Unix... it might break win32 in ispell, since ut_types
 * is no longer included.  This is a temporary solution to a larger problem
 * of including C++ headers in C source files.
 *
 * Revision 1.6  1999/01/07 05:14:22  sterwill
 * So it builds on Unix... it might break win32 in ispell, since ut_types
 * is no longer included.  This is a temporary solution to a larger problem
 * of including C++ headers in C source files.
 *
 * Revision 1.5  1999/01/07 05:02:25  sterwill
 * Checking in half-broken to avoid tree lossage
 *
 * Revision 1.4  1999/01/07 01:07:48  paul
 * Fixed spell leaks.
 *
 * Revision 1.3  1998/12/29 15:03:54  eric
 *
 * minor fix to ispell.h to get things to compile on Linux again.
 *
 * Revision 1.2  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.1  1998/12/28 18:04:43  davet
 * Spell checker code stripped from ispell.  At this point, there are
 * two external routines...  the Init routine, and a check-a-word routine
 * which returns a boolean value, and takes a 16 bit char string.
 * The code resembles the ispell code as much as possible still.
 *
 * Revision 1.68  1995/03/06  02:42:41  geoff
 * Be vastly more paranoid about parenthesizing macro arguments.  This
 * fixes a bug in defmt.c where a complex argument was passed to
 * isstringch.
 *
 * Revision 1.67  1995/01/03  19:24:12  geoff
 * Get rid of a non-global declaration.
 *
 * Revision 1.66  1994/12/27  23:08:49  geoff
 * Fix a lot of subtly bad assumptions about the widths of ints and longs
 * which only show up on 64-bit machines like the Cray and the DEC Alpha.
 *
 * Revision 1.65  1994/11/02  06:56:10  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.64  1994/10/25  05:46:18  geoff
 * Add the FF_ANYWORD flag for defining an affix that will apply to any
 * word, even if not explicitly specified.  (Good for French.)
 *
 * Revision 1.63  1994/09/16  04:48:28  geoff
 * Make stringdups and laststringch unsigned ints, and dupnos a plain
 * int, so that we can handle more than 128 stringchars and stringchar
 * types.
 *
 * Revision 1.62  1994/09/01  06:06:39  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.61  1994/08/31  05:58:35  geoff
 * Add contextoffset, used in -a mode to handle extremely long lines.
 *
 * Revision 1.60  1994/05/17  06:44:15  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.59  1994/03/15  06:25:16  geoff
 * Change deftflag's initialization so we can tell if -t/-n appeared.
 *
 * Revision 1.58  1994/02/07  05:53:28  geoff
 * Add typecasts to the the 7-bit versions of ichar* routines
 *
 * Revision 1.57  1994/01/25  07:11:48  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

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
#define MAXSTRINGCHARS 128
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

#ifdef NO8BIT
#define SET_SIZE	128
#else
#define SET_SIZE	256
#endif

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

/*
** Data type for internal word storage.  If necessary, we use shorts rather
** than chars so that string characters can be encoded as a single unit.
*/
#if (SET_SIZE + MAXSTRINGCHARS) <= 256
#ifndef lint
#define ICHAR_IS_CHAR
#endif /* lint */
#endif

#ifdef ICHAR_IS_CHAR
typedef unsigned char	ichar_t;	/* Internal character */
#define icharlen(s)	strlen ((char *) (s))
#define icharcpy(a, b)	strcpy ((char *) (a), (char *) (b))
#define icharcmp(a, b)	strcmp ((char *) (a), (char *) (b))
#define icharncmp(a, b, n) strncmp ((char *) (a), (char *) (b), (n))
#define chartoichar(x)	((ichar_t) (x))
#else
typedef unsigned short	ichar_t;	/* Internal character */
#define chartoichar(x)	((ichar_t) (unsigned char) (x))

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

#endif

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
#ifndef NO_CAPITALIZATION_SUPPORT
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
#endif
*/
#ifdef FULLMASKSET
#define flagfield	flags
#else
#define flagfield	mask[1]
#endif
#define USED		((MASKTYPE) 1 << (FLAGBASE + 0))
#define KEEP		((MASKTYPE) 1 << (FLAGBASE + 1))
#ifdef NO_CAPITALIZATION_SUPPORT
#define ALLFLAGS	(USED | KEEP)
#else /* NO_CAPITALIZATION_SUPPORT */
#define ANYCASE		((MASKTYPE) 0 << (FLAGBASE + 2))
#define ALLCAPS		((MASKTYPE) 1 << (FLAGBASE + 2))
#define CAPITALIZED	((MASKTYPE) 2 << (FLAGBASE + 2))
#define FOLLOWCASE	((MASKTYPE) 3 << (FLAGBASE + 2))
#define CAPTYPEMASK	((MASKTYPE) 3 << (FLAGBASE + 2))
#define MOREVARIANTS	((MASKTYPE) 1 << (FLAGBASE + 4))
#define ALLFLAGS	(USED | KEEP | CAPTYPEMASK | MOREVARIANTS)
#define captype(x)	((x) & CAPTYPEMASK)
#endif /* NO_CAPITALIZATION_SUPPORT */

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
#ifdef NO8BIT
# define MAGIC8BIT		0x01
#else
# define MAGIC8BIT		0x00
#endif
#ifdef NO_CAPITALIZATION_SUPPORT
# define MAGICCAPITALIZATION	0x00
#else
# define MAGICCAPITALIZATION	0x02
#endif
#  define MAGICMASKSET		0x04

#if MASKBITS <= 32
# define MAGICMASKSET		0x00
#else
# if MASKBITS <= 64
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

#endif /* ISPELL_H */
