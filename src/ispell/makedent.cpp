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

/*
 * Copyright 1988, 1989, 1992, 1993, Geoff Kuenning, Granada Hills, CA
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
 * Revision 1.4  2003/08/14 17:51:28  dom
 * update license - exception clause should be Lesser GPL
 *
 * Revision 1.3  2003/07/28 20:40:27  dom
 * fix up the license clause, further win32-registry proof some directory getting functions
 *
 * Revision 1.2  2003/07/16 22:52:49  dom
 * LGPL + exception license
 *
 * Revision 1.1  2003/07/15 01:15:08  dom
 * ispell enchant backend
 *
 * Revision 1.3  2003/02/12 02:10:38  hippietrail
 *
 * C casts -> C++ casts
 * Improved const-correctness due to changing casts
 * Fixed some warnings
 *
 * Revision 1.2  2003/01/29 05:50:12  hippietrail
 *
 * Fixed my mess in EncodingManager.
 * Changed many C casts to C++ casts.
 *
 * Revision 1.1  2003/01/24 05:52:35  hippietrail
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
 * Revision 1.8  2003/01/06 18:48:40  dom
 * ispell cleanup, start of using new 'add' save features
 *
 * Revision 1.7  2003/01/04 19:09:04  dom
 * some tidying... bug pissing me off...
 *
 * Revision 1.6  2002/09/19 05:31:18  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.5  2002/09/17 03:03:30  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.4  2002/09/13 17:20:13  mpritchett
 * Fix more warnings for Linux build
 *
 * Revision 1.3  2002/03/22 14:31:57  dom
 * fix mg's compile problem
 *
 * Revision 1.2  2001/05/12 16:05:42  thomasf
 * Big pseudo changes to ispell to make it pass around a structure rather
 * than rely on all sorts of gloabals willy nilly here and there.  Also
 * fixed our spelling class to work with accepting suggestions once more.
 * This code is dirty, gross and ugly (not to mention still not supporting
 * multiple hash sized just yet) but it works on my machine and will no
 * doubt break other machines.
 *
 * Revision 1.1  2001/04/15 16:01:24  tomas_f
 * moving to spell/xp
 *
 * Revision 1.6  1999/12/21 18:46:29  sterwill
 * ispell patch for non-English dictionaries by Henrik Berg <henrik@lansen.se>
 *
 * Revision 1.5  1999/10/20 03:19:35  paul
 * Hacked ispell code to ignore any characters that don't fit in the lookup tables loaded from the dictionary.  It ain't pretty, but at least we don't crash there any more.
 *
 * Revision 1.4  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.3  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.3  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.2  1998/12/28 23:11:30  eric
 *
 * modified spell code and integration to build on Windows.
 * This is still a hack.
 *
 * Actually, it doesn't yet WORK on Windows.  It just builds.
 * SpellCheckInit is failing for some reason.
 *
 * Revision 1.1  1998/12/28 18:04:43  davet
 * Spell checker code stripped from ispell.  At this point, there are
 * two external routines...  the Init routine, and a check-a-word routine
 * which returns a boolean value, and takes a 16 bit char string.
 * The code resembles the ispell code as much as possible still.
 *
 * Revision 1.45  1994/12/27  23:08:52  geoff
 * Add code to makedent to reject words that contain non-word characters.
 * This helps protect people who use ISO 8-bit characters when ispell
 * isn't configured for that option.
 *
 * Revision 1.44  1994/10/25  05:46:20  geoff
 * Fix some incorrect declarations in the lint versions of some routines.
 *
 * Revision 1.43  1994/09/16  03:32:34  geoff
 * Issue an error message for bad affix flags
 *
 * Revision 1.42  1994/02/07  04:23:43  geoff
 * Correctly identify the deformatter when changing file types
 *
 * Revision 1.41  1994/01/25  07:11:55  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ispell_checker.h"
#include "msgs.h"

int		makedent P ((char * lbuf, int lbuflen, struct dent * ent));
/*int		combinecaps P ((struct dent * hdr, struct dent * newent));
#ifndef NO_CAPITALIZATION_SUPPORT
static void	forcevheader P ((struct dent * hdrp, struct dent * oldp,
		  struct dent * newp));
#endif / * NO_CAPITALIZATION_SUPPORT * /
static int	combine_two_entries P ((struct dent * hdrp,
		  struct dent * oldp, struct dent * newp));
static int	acoversb P ((struct dent * enta, struct dent * entb));
*/
/*static int	issubset P ((struct dent * ent1, struct dent * ent2));
static void	combineaffixes P ((struct dent * ent1, struct dent * ent2));*/

void		toutent P ((FILE * outfile, struct dent * hent,
		  int onlykeep));
/*static void	toutword P ((FILE * outfile, char * word,
		  struct dent * cent));
static void	flagout P ((FILE * outfile, int flag));
*/
#ifndef ICHAR_IS_CHAR
ichar_t *	icharcpy P ((ichar_t * out, ichar_t * in));
int		icharlen P ((ichar_t * str));
int		icharcmp P ((ichar_t * s1, ichar_t * s2));
int		icharncmp P ((ichar_t * s1, ichar_t * s2, int n));
#endif /* ICHAR_IS_CHAR */

/*static int  	has_marker;*/

/*
 * Fill in a directory entry, including setting the capitalization flags, and
 * allocate and initialize memory for the d->word field.  Returns -1
 * if there was trouble.  The input word must be in canonical form.
int makedent (lbuf, lbuflen, d)
This function is not used by AbiWord.  I don't know if it'll be needed for 
other abi documents
 */
	
#ifndef NO_CAPITALIZATION_SUPPORT
/*!
** Classify the capitalization of a sample entry.  Returns one of the
** four capitalization codes ANYCASE, ALLCAPS, CAPITALIZED, or FOLLOWCASE.
**
** \param word
**
** \return
*/
long
ISpellChecker::whatcap (ichar_t *word)
{
    register ichar_t *	p;

    for (p = word;  *p;  p++)
	{
		if (mylower (*p))
			break;
	}
    if (*p == '\0')
		return ALLCAPS;
    else
	{
		for (  ;  *p;  p++)
	    {
			if (myupper (*p))
				break;
	    }
		if (*p == '\0')
	    {
			/*
			** No uppercase letters follow the lowercase ones.
			** If there is more than one uppercase letter, it's
			** "followcase". If only the first one is capitalized,
			** it's "capitalize".  If there are no capitals
			** at all, it's ANYCASE.
			*/
			if (myupper (word[0]))
			{
				for (p = word + 1;  *p != '\0';  p++)
				{
					if (myupper (*p))
						return FOLLOWCASE;
				}
				return CAPITALIZED;
			}
			else
				return ANYCASE;
	    }
		else
			return FOLLOWCASE;	/* .../lower/upper */
	}
}

/*!
** Add a variant-capitalization header to a word.  This routine may be
** called even for a followcase word that doesn't yet have a header.
**
** \param dp Entry to update
**
** \return 0 if all was ok, -1 if allocation error.
*/
int ISpellChecker::addvheader ( struct dent *dp)
{
    register struct dent *	tdent; /* Copy of entry */

    /*
    ** Add a second entry with the correct capitalization, and then make
    ** dp into a special dummy entry.
    */
    tdent = static_cast<struct dent *>(malloc(sizeof (struct dent)));
    if (tdent == NULL)
	{
		fprintf (stderr, MAKEDENT_C_NO_WORD_SPACE, dp->word);
		return -1;
	}
    *tdent = *dp;
    if (captype (tdent->flagfield) != FOLLOWCASE)
		tdent->word = NULL;
    else
	{
		/* Followcase words need a copy of the capitalization */
		tdent->word = static_cast<char *>(malloc (static_cast<unsigned int>(strlen(tdent->word)) + 1));
		if (tdent->word == NULL)
	    {
			fprintf (stderr, MAKEDENT_C_NO_WORD_SPACE, dp->word);
			free (reinterpret_cast<char *>(tdent));
			return -1;
	    }
		strcpy (tdent->word, dp->word);
	}
    chupcase (dp->word);
    dp->next = tdent;
    dp->flagfield &= ~CAPTYPEMASK;
    dp->flagfield |= (ALLCAPS | MOREVARIANTS);
    return 0;
}
#endif /* NO_CAPITALIZATION_SUPPORT */

/*
** Combine and resolve the entries describing two capitalizations of the same
** word.  This may require allocating yet more entries.
**
** Hdrp is a pointer into a hash table.  If the word covered by hdrp has
** variations, hdrp must point to the header.  Newp is a pointer to temporary
** storage, and space is malloc'ed if newp is to be kept.  The newp->word
** field must have been allocated with mymalloc, so that this routine may free
** the space if it keeps newp but not the word.
**
** Return value:  0 if the word was added, 1 if the word was combined
** with an existing entry, and -1 if trouble occurred (e.g., malloc).
** If 1 is returned, newp->word may have been be freed using myfree.
**
** Life is made much more difficult by the KEEP flag's possibilities.  We
** must ensure that a !KEEP word doesn't find its way into the personal
** dictionary as a result of this routine's actions.  However, a !KEEP
** word that has affixes must have come from the main dictionary, so it
** is acceptable to combine entries in that case (got that?).
**
** The net result of all this is a set of rules that is a bloody pain
** to figure out.  Basically, we want to choose one of the following actions:
**
**	(1) Add newp's affixes and KEEP flag to oldp, and discard newp.
**	(2) Add oldp's affixes and KEEP flag to newp, replace oldp with
**	    newp, and discard newp.
#ifndef NO_CAPITALIZATION_SUPPORT
**	(3) Insert newp as a new entry in the variants list.  If there is
**	    currently no variant header, this requires adding one.  Adding a
**	    header splits into two sub-cases:
**
**	    (3a) If oldp is ALLCAPS and the KEEP flags match, just turn it
**		into the header.
**	    (3b) Otherwise, add a new entry to serve as the header.
**		To ease list linking, this is done by copying oldp into
**		the new entry, and then performing (3a).
**
**	    After newp has been added as a variant, its affixes and KEEP
**	    flag are OR-ed into the variant header.
#endif
**
** So how to choose which?  The default is always case (3), which adds newp
** as a new entry in the variants list.  Cases (1) and (2) are symmetrical
** except for which entry is discarded.  We can use case (1) or (2) whenever
** one entry "covers" the other.  "Covering" is defined as follows:
**
**	(4) For entries with matching capitalization types, A covers B
**	    if:
**
**	    (4a) B's affix flags are a subset of A's, or the KEEP flags
**		 match, and
**	    (4b) either the KEEP flags match, or A's KEEP flag is set.
**		(Since A has more suffixes, combining B with it won't
**		cause any extra suffixes to be added to the dictionary.)
**	    (4c) If the words are FOLLOWCASE, the capitalizations match
**		exactly.
**
#ifndef NO_CAPITALIZATION_SUPPORT
**	(5) For entries with mismatched capitalization types, A covers B
**	    if (4a) and (4b) are true, and:
**
**	    (5a) B is ALLCAPS, or
**	    (5b) A is ANYCASE, and B is CAPITALIZED.
#endif
**
** For any "hdrp" without variants, oldp is the same as hdrp.  Otherwise,
** the above tests are applied using each variant in turn for oldp.
int combinecaps (hdrp, newp)
static void forcevheader (hdrp, oldp, newp)
static int combine_two_entries (hdrp, oldp, newp)
static int acoversb (enta, entb)
*/

/*
 * \param s
 */
void
ISpellChecker::upcase (ichar_t *s)
{

    while (*s)
	{
		*s = mytoupper (*s);
		s++;
	}
}

/*
 * \param s
 */
void
ISpellChecker::lowcase (ichar_t *s)
{

    while (*s)
	{
		*s = mytolower (*s);
		s++;
	}
}

/*!
 * Upcase variant that works on normal strings.  Note that it is a lot
 * slower than the normal upcase.  The input must be in canonical form.
 * 
 * \param s
 */
void
ISpellChecker::chupcase (char *s)
{
    ichar_t *	is;

    is = strtosichar (s, 1);
    upcase (is);
    ichartostr (s, is, strlen (s) + 1, 1);
}

/*
** See if one affix field is a subset of another.  Returns NZ if ent1
** is a subset of ent2.  The KEEP flag is not taken into consideration.
static int issubset (ent1, ent2)
static void combineaffixes (ent1, ent2)
*/

/*
** Write out a dictionary entry, including capitalization variants.
** If onlykeep is true, only those variants with KEEP set will be
** written.
Removed -- not used by Abiword
void toutent_ (toutfile, hent, onlykeep)
static void toutword (toutfile, word, cent)
static void flagout (toutfile, flag)
*/

/*!
 * If the string under the given pointer begins with a string character,
 * return the length of that "character".  If not, return 0.
 * May be called any time, but it's best if "isstrstart" is first
 * used to filter out unnecessary calls.
 *
 * As a side effect, "laststringch" is set to the number of the string
 * found, or to -1 if none was found.  This can be useful for such things
 * as case conversion.
 *
 * \param bufp
 * \param canonical NZ if input is in canonical form
 *
 * \return
 */
int
ISpellChecker::stringcharlen (char *bufp, int canonical)
{
#ifdef SLOWMULTIPLY
    static char *	sp[MAXSTRINGCHARS];
    static int		inited = 0;
#endif /* SLOWMULTIPLY */
    register char *	bufcur;
    register char *	stringcur;
    register int	stringno;
    register int	lowstringno;
    register int	highstringno;
    int			dupwanted;

#ifdef SLOWMULTIPLY
    if (!inited)
	{
		inited = 1;
		for (stringno = 0;  stringno < MAXSTRINGCHARS;  stringno++)
			sp[stringno] = &hashheader.stringchars[stringno][0];
	}
#endif /* SLOWMULTIPLY */
    lowstringno = 0;
    highstringno = m_hashheader.nstrchars - 1;
    dupwanted = canonical ? 0 : m_defdupchar;
    while (lowstringno <= highstringno)
	{
		stringno = (lowstringno + highstringno) >> 1;
#ifdef SLOWMULTIPLY
		stringcur = sp[stringno];
#else /* SLOWMULTIPLY */
		stringcur = &m_hashheader.stringchars[stringno][0];
#endif /* SLOWMULTIPLY */
		bufcur = bufp;
		while (*stringcur)
	    {
#ifdef NO8BIT
			if (((*bufcur++ ^ *stringcur) & 0x7F) != 0)
#else /* NO8BIT */
			if (*bufcur++ != *stringcur)
#endif /* NO8BIT */
				break;
			/*
			** We can't use autoincrement above because of the
			** test below.
			*/
			stringcur++;
	    }
		if (*stringcur == '\0')
	    {
			if (m_hashheader.dupnos[stringno] == dupwanted)
			{
				/* We have a match */
				m_laststringch = m_hashheader.stringdups[stringno];
#ifdef SLOWMULTIPLY
				return stringcur - sp[stringno];
#else /* SLOWMULTIPLY */
				return stringcur - &m_hashheader.stringchars[stringno][0];
#endif /* SLOWMULTIPLY */
			}
			else
				--stringcur;
	    }
		/* No match - choose which side to search on */
#ifdef NO8BIT
		if ((*--bufcur & 0x7F) < (*stringcur & 0x7F))
			highstringno = stringno - 1;
		else if ((*bufcur & 0x7F) > (*stringcur & 0x7F))
			lowstringno = stringno + 1;
#else /* NO8BIT */
		if (*--bufcur < *stringcur)
			highstringno = stringno - 1;
		else if (*bufcur > *stringcur)
			lowstringno = stringno + 1;
#endif /* NO8BIT */
		else if (dupwanted < m_hashheader.dupnos[stringno])
			highstringno = stringno - 1;
		else
			lowstringno = stringno + 1;
	}
    m_laststringch = static_cast<unsigned int>(-1);
    return 0;			/* Not a string character */
}

/* MACROS CONVERTED TO FUNCTIONS
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
#define isstringch(ptr, canon)	(isstringstart (*(ptr)) \
				  &&  stringcharlen ((ptr), (canon)) > 0)
/*
int isstringch(char *ptr, int canon) {
	return (isstringstart (*(ptr)) && (len = stringcharlen ((ptr), (canon))) > 0);
}
*/

#define l_isstringch(ptr, len, canon)	\
				(isstringstart (*(ptr)) \
				  &&  (len = stringcharlen ((ptr), (canon))) \
				    > 0)
/*
int l_isstringch(char *ptr, int len, int canon) {
	return (isstringstart (*(ptr)) &&  (len = stringcharlen ((ptr), (canon))) > 0);
}
*/

#define l1_isstringch(ptr, len, canon)	\
				(len = 1, \
				  isstringstart ((unsigned char)(*(ptr))) \
				    &&  ((len = \
					  stringcharlen ((ptr), (canon))) \
					> 0 \
				      ? 1 : (len = 1, 0)))
/*
int l1_isstringch(char *ptr, int len, int canon) {
	return (len = 1, isstringstart ((unsigned char)(*(ptr))) &&  
           ((len = stringcharlen ((ptr), (canon))) > 0 ? 1 : (len = 1, 0)));
}
*/

/*** END MACRO CONVERSION ***/

/*!
 * Convert an external string to an ichar_t string.  If necessary, the parity
 * bit is stripped off as part of the process.
 *
 * \param out Where to put result
 * \param in String to convert
 * \param outlen Size of output buffer, *BYTES*
 * \param canonical NZ if input is in canonical form
 *
 * \return NZ if the output string overflowed.
 */
int
ISpellChecker::strtoichar (ichar_t *out, char *in, int outlen, int canonical)
{
    register int len = 1;		/* Length of next character */

    outlen /= sizeof (ichar_t);		/* Convert to an ichar_t count */
    for (  ;  --outlen > 0  &&  *in != '\0';  in += len)
	{
		if (l1_isstringch (in, len , canonical))
			*out++ = SET_SIZE + m_laststringch;
		else
			*out++ = (unsigned char)( *in );
	}
    *out = 0;
    return outlen <= 0;
}

/*!
 * Convert an ichar_t string to an external string.
 *
 * WARNING: the resulting string may wind up being longer than the
 * original.  In fact, even the sequence strtoichar->ichartostr may
 * produce a result longer than the original, because the output form
 * may use a different string type set than the original input form.
 *
 * \param out Where to put result
 * \param in String to convert
 * \param outlen Size of output buffer, bytes
 * \param canonical NZ for canonical form
 *
 * \return NZ if the output string overflowed.
 */
int
ISpellChecker::ichartostr ( char *out, ichar_t *in, int outlen, int canonical)
{
    register int	ch;		/* Next character to store */
    register int	i;		/* Index into duplicates list */
    register char *	scharp;		/* Pointer into a string char */

    while (--outlen > 0  &&  (ch = *in++) != 0)
	{
		if (ch < SET_SIZE)
			*out++ = static_cast<char>(ch);
		else
		{
			ch -= SET_SIZE;
			if (!canonical)
			{
				for (i = m_hashheader.nstrchars;  --i >= 0;  )
				{
					if (m_hashheader.dupnos[i] == m_defdupchar
					  &&  (static_cast<int>(m_hashheader.stringdups[i])) == ch)
					{
						ch = i;
						break;
					}
				}
			}
			scharp = m_hashheader.stringchars[static_cast<unsigned>(ch)];
			while ((*out++ = *scharp++) != '\0')
				;
			out--;
	    }
	}
    *out = '\0';
    return outlen <= 0;
}

/*!
 * Convert a string to an ichar_t, storing the result in a static area.
 *
 * \param in String to convert
 * \param canonical NZ if input is in canonical form
 *
 * \return
 */
ichar_t *
ISpellChecker::strtosichar ( char *in, int canonical)
{
    static ichar_t	out[STRTOSICHAR_SIZE / sizeof (ichar_t)];

    if (strtoichar (out, in, sizeof out, canonical))
		fprintf (stderr, WORD_TOO_LONG (in));
    return out;
}

/*!
 * Convert an ichar_t to a string, storing the result in a static area.
 *
 * \param in Internal string to convert
 * \param canonical NZ for canonical conversion
 *
 * \return
 */
char *
ISpellChecker::ichartosstr (ichar_t *in, int canonical)
{
    static char		out[ICHARTOSSTR_SIZE];

    if (ichartostr (out, in, sizeof out, canonical))
		fprintf (stderr, WORD_TOO_LONG (out));
    return out;
}

/*!
 * Convert a single ichar to a printable string, storing the result in
 * a static area.
 *
 * \param in
 *
 * \return
 */
char *
ISpellChecker::printichar (int in)
{
    static char		out[MAXSTRINGCHARLEN + 1];

    if (in < SET_SIZE)
	{
		out[0] = static_cast<char>(in);
		out[1] = '\0';
	}
    else
		strcpy (out, m_hashheader.stringchars[static_cast<unsigned>(in) - SET_SIZE]);
    return out;
}

#ifndef ICHAR_IS_CHAR
/*!
 * Copy an ichar_t.
 *
 * \param out Destination
 * \param in Source
 *
 * \return
 */
ichar_t *
icharcpy (ichar_t *out, ichar_t *in)
{
    ichar_t *		origout;	/* Copy of destination for return */

    origout = out;
    while ((*out++ = *in++) != 0)
		;
    return origout;
}

/*!
 * Return the length of an ichar_t.
 *
 * \param in String to count
 *
 * \return
 */
int
icharlen (ichar_t * in)
{
    register int	len;		/* Length so far */

    for (len = 0;  *in++ != 0;  len++)
		;
    return len;
}

/*!
 * Compare two ichar_t's.
 *
 * \param s1
 * \param s2
 *
 * \return
 */
int
icharcmp (ichar_t * s1, ichar_t * s2)
{

    while (*s1 != 0)
	{
		if (*s1++ != *s2++)
			return *--s1 - *--s2;
	}
    return *s1 - *s2;
}

/*!
 * Strncmp for two ichar_t's.
 *
 * \param s1
 * \param s2
 * \param n
 *
 * \return
 */
int
icharncmp (ichar_t *s1, ichar_t *s2, int n)
{

    while (--n >= 0  &&  *s1 != 0)
	{
		if (*s1++ != *s2++)
			return *--s1 - *--s2;
	}
    if (n < 0)
		return 0;
    else
		return *s1 - *s2;
}

#endif /* ICHAR_IS_CHAR */

/*
 * \param istate
 * \param name
 * \param searchnames
 * \param deformatter
 *
 * \return
 */
int
ISpellChecker::findfiletype (const char *name, int searchnames, int *deformatter)
{
    char *		cp;		/* Pointer into suffix list */
    int			cplen;		/* Length of current suffix */
    register int	i;		/* Index into type table */
    int			len;		/* Length of the name */

    /*
     * Note:  for now, the deformatter is set to 1 for tex, 0 for nroff.
     * Further, we assume that it's one or the other, so that a test
     * for tex is sufficient.  This needs to be generalized.
     */
    len = strlen (name);
    if (searchnames)
	{
		for (i = 0;  i < m_hashheader.nstrchartype;  i++)
	    {
			if (strcmp (name, m_chartypes[i].name) == 0)
			{
				if (deformatter != NULL)
					*deformatter =
					  (strcmp (m_chartypes[i].deformatter, "tex") == 0);
				return i;
			}
	    }
	}
    for (i = 0;  i < m_hashheader.nstrchartype;  i++)
	{
		for (cp = m_chartypes[i].suffixes;  *cp != '\0';  cp += cplen + 1)
		{
			cplen = strlen (cp);
			if (len >= cplen  &&  strcmp (&name[len - cplen], cp) == 0)
			{
				if (deformatter != NULL)
					*deformatter =
					  (strcmp (m_chartypes[i].deformatter, "tex") == 0);
				return i;
			}
	    }
	}
    return -1;
}

/*
	HACK: macros replaced with function implementations 
	so we could do a side-effect-free check for unicode
	characters which aren't in hashheader

	TODO: this is just a workaround to keep us from crashing. 
	more sophisticated logic needed here. 
*/
char ISpellChecker::myupper(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.upperchars[c];
	else
		return 0;
}

char ISpellChecker::mylower(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.lowerchars[c];
	else
		return 0;
}

int myspace(ichar_t c)
{
	return ((c > 0)  &&  (c < 0x80) &&  isspace(static_cast<unsigned char>(c)));
}

char ISpellChecker::iswordch(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.wordchars[c];
	else
		return 0;
}

char ISpellChecker::isboundarych(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.boundarychars[c];
	else
		return 0;
}

char ISpellChecker::isstringstart(ichar_t c)
{
	if (c < (SET_SIZE))
		return m_hashheader.stringstarts[static_cast<unsigned char>(c)];
	else
		return 0;
}

ichar_t ISpellChecker::mytolower(ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.lowerconv[c];
	else
		return c;
}

ichar_t ISpellChecker::mytoupper (ichar_t c)
{
	if (c < (SET_SIZE + MAXSTRINGCHARS))
		return m_hashheader.upperconv[c];
	else
		return c;
}

