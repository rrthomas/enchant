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
 * correct.c - Routines to manage the higher-level aspects of spell-checking
 *
 * This code originally resided in ispell.c, but was moved here to keep
 * file sizes smaller.
 *
 * Copyright (c), 1983, by Pace Willisson
 *
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
 * Revision 1.4  2003/08/14 17:51:26  dom
 * update license - exception clause should be Lesser GPL
 *
 * Revision 1.3  2003/07/28 20:40:25  dom
 * fix up the license clause, further win32-registry proof some directory getting functions
 *
 * Revision 1.2  2003/07/16 22:52:35  dom
 * LGPL + exception license
 *
 * Revision 1.1  2003/07/15 01:15:04  dom
 * ispell enchant backend
 *
 * Revision 1.2  2003/01/29 05:50:11  hippietrail
 *
 * Fixed my mess in EncodingManager.
 * Changed many C casts to C++ casts.
 *
 * Revision 1.1  2003/01/24 05:52:31  hippietrail
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
 * Revision 1.7  2002/09/19 05:31:15  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.6  2002/09/17 03:03:28  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.5  2002/09/13 17:20:12  mpritchett
 * Fix more warnings for Linux build
 *
 * Revision 1.4  2002/03/06 08:27:16  fjfranklin
 * o Only activate compound handling when the hash file says so (Per Larsson)
 *
 * Revision 1.3  2001/05/14 09:52:50  hub
 * Removed newMain.c from GNUmakefile.am
 *
 * C++ comments are not C comment. Changed to C comments
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
 * Revision 1.2  1999/10/05 16:17:28  paul
 * Fixed build, and other tidyness.
 * Spell dialog enabled by default, with keyboard binding of F7.
 *
 * Revision 1.1  1999/09/29 23:33:32  justin
 * Updates to the underlying ispell-based code to support suggested corrections.
 *
 * Revision 1.59  1995/08/05  23:19:43  geoff
 * Fix a bug that caused offsets for long lines to be confused if the
 * line started with a quoting uparrow.
 *
 * Revision 1.58  1994/11/02  06:56:00  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.57  1994/10/26  05:12:39  geoff
 * Try boundary characters when inserting or substituting letters, except
 * (naturally) at word boundaries.
 *
 * Revision 1.56  1994/10/25  05:46:30  geoff
 * Fix an assignment inside a conditional that could generate spurious
 * warnings (as well as being bad style).  Add support for the FF_ANYWORD
 * option.
 *
 * Revision 1.55  1994/09/16  04:48:24  geoff
 * Don't pass newlines from the input to various other routines, and
 * don't assume that those routines leave the input unchanged.
 *
 * Revision 1.54  1994/09/01  06:06:41  geoff
 * Change erasechar/killchar to uerasechar/ukillchar to avoid
 * shared-library problems on HP systems.
 *
 * Revision 1.53  1994/08/31  05:58:38  geoff
 * Add code to handle extremely long lines in -a mode without splitting
 * words or reporting incorrect offsets.
 *
 * Revision 1.52  1994/05/25  04:29:24  geoff
 * Fix a bug that caused line widths to be calculated incorrectly when
 * displaying lines containing tabs.  Fix a couple of places where
 * characters were sign-extended incorrectly, which could cause 8-bit
 * characters to be displayed wrong.
 *
 * Revision 1.51  1994/05/17  06:44:05  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.50  1994/04/27  05:20:14  geoff
 * Allow compound words to be formed from more than two components
 *
 * Revision 1.49  1994/04/27  01:50:31  geoff
 * Add support to correctly capitalize words generated as a result of a
 * missing-space suggestion.
 *
 * Revision 1.48  1994/04/03  23:23:02  geoff
 * Clean up the code in missingspace() to be a bit simpler and more
 * efficient.
 *
 * Revision 1.47  1994/03/15  06:24:23  geoff
 * Fix the +/-/~ commands to be independent.  Allow the + command to
 * receive a suffix which is a deformatter type (currently hardwired to
 * be either tex or nroff/troff).
 *
 * Revision 1.46  1994/02/21  00:20:03  geoff
 * Fix some bugs that could cause bad displays in the interaction between
 * TeX parsing and string characters.  Show_char now will not overrun
 * the inverse-video display area by accident.
 *
 * Revision 1.45  1994/02/14  00:34:51  geoff
 * Fix correct to accept length parameters for ctok and itok, so that it
 * can pass them to the to/from ichar routines.
 *
 * Revision 1.44  1994/01/25  07:11:22  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ispell_checker.h"
#include "msgs.h"

/*
extern void upcase P ((ichar_t * string));
extern void lowcase P ((ichar_t * string));
extern ichar_t * strtosichar P ((char * in, int canonical));

int compoundflag = COMPOUND_CONTROLLED;
*/

/*
 * \param a
 * \param b
 * \param canonical NZ for canonical string chars
 *
 * \return
 */
int
ISpellChecker::casecmp (char *a, char *b, int canonical)
{
    register ichar_t *	ap;
    register ichar_t *	bp;
    ichar_t		inta[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
    ichar_t		intb[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];

    strtoichar (inta, a, sizeof inta, canonical);
    strtoichar (intb, b, sizeof intb, canonical);
    for (ap = inta, bp = intb;  *ap != 0;  ap++, bp++)
	{
		if (*ap != *bp)
	    {
			if (*bp == '\0')
				return m_hashheader.sortorder[*ap];
			else if (mylower (*ap))
			{
				if (mylower (*bp)  ||  mytoupper (*ap) != *bp)
					return static_cast<int>(m_hashheader.sortorder[*ap])
					  - static_cast<int>(m_hashheader.sortorder[*bp]);
			}
			else
			{
				if (myupper (*bp)  ||  mytolower (*ap) != *bp)
					return static_cast<int>(m_hashheader.sortorder[*ap])
					  - static_cast<int>(m_hashheader.sortorder[*bp]);
			}
	    }
	}
    if (*bp != '\0')
		return -static_cast<int>(m_hashheader.sortorder[*bp]);
    for (ap = inta, bp = intb;  *ap;  ap++, bp++)
	{
		if (*ap != *bp)
	    {
			return static_cast<int>(m_hashheader.sortorder[*ap])
			  - static_cast<int>(m_hashheader.sortorder[*bp]);
	    }
	}
    return 0;
}

/*
 * \param word
 */
void
ISpellChecker::makepossibilities (ichar_t *word)
{
    register int	i;

    for (i = 0; i < MAXPOSSIBLE; i++)
	m_possibilities[i][0] = 0;
    m_pcount = 0;
    m_maxposslen = 0;
    m_easypossibilities = 0;

#ifndef NO_CAPITALIZATION_SUPPORT
    wrongcapital (word);
#endif

/* 
 * according to Pollock and Zamora, CACM April 1984 (V. 27, No. 4),
 * page 363, the correct order for this is:
 * OMISSION = TRANSPOSITION > INSERTION > SUBSTITUTION
 * thus, it was exactly backwards in the old version. -- PWP
 */

    if (m_pcount < MAXPOSSIBLE)
		missingletter (word);		/* omission */
    if (m_pcount < MAXPOSSIBLE)
		transposedletter (word);	/* transposition */
    if (m_pcount < MAXPOSSIBLE)
		extraletter (word);		/* insertion */
    if (m_pcount < MAXPOSSIBLE)
		wrongletter (word);		/* substitution */

    if ((m_hashheader.compoundflag != COMPOUND_ANYTIME)  &&
		  m_pcount < MAXPOSSIBLE)
		missingspace (word);	/* two words */

}

/*
 * \param word
 *
 * \return
 */
int
ISpellChecker::insert (ichar_t *word)
{
    register int	i;
    register char *	realword;

    realword = ichartosstr (word, 0);
    for (i = 0; i < m_pcount; i++)
	{
		if (strcmp (m_possibilities[i], realword) == 0)
			return (0);
	}

    strcpy (m_possibilities[m_pcount++], realword);
    i = strlen (realword);
    if (i > m_maxposslen)
		m_maxposslen = i;
    if (m_pcount >= MAXPOSSIBLE)
		return (-1);
    else
		return (0);
}

#ifndef NO_CAPITALIZATION_SUPPORT
/*
 * \param word
 */
void
ISpellChecker::wrongcapital (ichar_t *word)
{
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    /*
    ** When the third parameter to "good" is nonzero, it ignores
    ** case.  If the word matches this way, "ins_cap" will recapitalize
    ** it correctly.
    */
    if (good (word, 0, 1, 0, 0))
	{
		icharcpy (newword, word);
		upcase (newword);
		ins_cap (newword, word);
	}
}
#endif

/*
 * \param word
 */
void
ISpellChecker::wrongletter (ichar_t *word)
{
    register int	i;
    register int	j;
    register int	n;
    ichar_t		savechar;
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];

    n = icharlen (word);
    icharcpy (newword, word);
#ifndef NO_CAPITALIZATION_SUPPORT
    upcase (newword);
#endif

    for (i = 0; i < n; i++)
	{
		savechar = newword[i];
		for (j=0; j < m_Trynum; ++j)
		{
			if (m_Try[j] == savechar)
				continue;
			else if (isboundarych (m_Try[j])  &&  (i == 0  ||  i == n - 1))
				continue;
			newword[i] = m_Try[j];
			if (good (newword, 0, 1, 0, 0))
			{
				if (ins_cap (newword, word) < 0)
					return;
			}
		}
		newword[i] = savechar;
	}
}

/*
 * \param word
 */
void
ISpellChecker::extraletter (ichar_t *word)
{
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t *	r;

    if (icharlen (word) < 2)
		return;

    icharcpy (newword, word + 1);
    for (p = word, r = newword;  *p != 0;  )
	{
		if (good (newword, 0, 1, 0, 0))
		{
			if (ins_cap (newword, word) < 0)
				return;
		}
		*r++ = *p++;
	}
}

/*
 * \param word
 */
void
ISpellChecker::missingletter (ichar_t *word)
{
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    register ichar_t *	p;
    register ichar_t *	r;
    register int	i;

    icharcpy (newword + 1, word);
    for (p = word, r = newword;  *p != 0;  )
	{
		for (i = 0;  i < m_Trynum;  i++)
	    {
			if (isboundarych (m_Try[i])  &&  r == newword)
				continue;
			*r = m_Try[i];
			if (good (newword, 0, 1, 0, 0))
			{
				if (ins_cap (newword, word) < 0)
					return;
			}
	    }
		*r++ = *p++;
	}
    for (i = 0;  i < m_Trynum;  i++)
	{
		if (isboundarych (m_Try[i]))
			continue;
		*r = m_Try[i];
		if (good (newword, 0, 1, 0, 0))
		{
			if (ins_cap (newword, word) < 0)
				return;
		}
	}
}

/*
 * \param word
 */
void ISpellChecker::missingspace (ichar_t *word)
{
    ichar_t		firsthalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			firstno;	/* Index into first */
    ichar_t *		firstp;		/* Ptr into current firsthalf word */
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN + 1];
    int			nfirsthalf;	/* No. words saved in 1st half */
    int			nsecondhalf;	/* No. words saved in 2nd half */
    register ichar_t *	p;
    ichar_t		secondhalf[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			secondno;	/* Index into second */

    /*
    ** We don't do words of length less than 3;  this keeps us from
    ** splitting all two-letter words into two single letters.  We
    ** also don't do maximum-length words, since adding the space
    ** would exceed the size of the "possibilities" array.
    */
    nfirsthalf = icharlen (word);
    if (nfirsthalf < 3  ||  nfirsthalf >= INPUTWORDLEN + MAXAFFIXLEN - 1)
		return;
    icharcpy (newword + 1, word);
    for (p = newword + 1;  p[1] != '\0';  p++)
	{
		p[-1] = *p;
		*p = '\0';
		if (good (newword, 0, 1, 0, 0))
		{
			/*
			 * Save_cap must be called before good() is called on the
			 * second half, because it uses state left around by
			 * good().  This is unfortunate because it wastes a bit of
			 * time, but I don't think it's a significant performance
			 * problem.
			 */
			nfirsthalf = save_cap (newword, word, firsthalf);
			if (good (p + 1, 0, 1, 0, 0))
			{
				nsecondhalf = save_cap (p + 1, p + 1, secondhalf);
				for (firstno = 0;  firstno < nfirsthalf;  firstno++)
				{
					firstp = &firsthalf[firstno][p - newword];
					for (secondno = 0;  secondno < nsecondhalf;  secondno++)
					{
						*firstp = ' ';
						icharcpy (firstp + 1, secondhalf[secondno]);
						if (insert (firsthalf[firstno]) < 0)
							return;
						*firstp = '-';
						if (insert (firsthalf[firstno]) < 0)
							return;
					}
				}
			}
		}
	}
}

/*
 * \param word
 * \param pfxopts Options to apply to prefixes
 */
int
ISpellChecker::compoundgood (ichar_t *word, int pfxopts)
{
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	savech;
    long		secondcap;	/* Capitalization of 2nd half */

    /*
    ** If compoundflag is COMPOUND_NEVER, compound words are never ok.
    */
    if (m_hashheader.compoundflag == COMPOUND_NEVER)
		return 0;
    /*
    ** Test for a possible compound word (for languages like German that
    ** form lots of compounds).
    **
    ** This is similar to missingspace, except we quit on the first hit,
    ** and we won't allow either member of the compound to be a single
    ** letter.
    **
    ** We don't do words of length less than 2 * compoundmin, since
    ** both halves must at least compoundmin letters.
    */
    if (icharlen (word) < 2 * m_hashheader.compoundmin)
		return 0;
    icharcpy (newword, word);
    p = newword + m_hashheader.compoundmin;
    for (  ;  p[m_hashheader.compoundmin - 1] != 0;  p++)
	{
		savech = *p;
		*p = 0;
		if (good (newword, 0, 0, pfxopts, FF_COMPOUNDONLY))
	    {
			*p = savech;
			if (good (p, 0, 1, FF_COMPOUNDONLY, 0)
			  ||  compoundgood (p, FF_COMPOUNDONLY))
			{
				secondcap = whatcap (p);
				switch (whatcap (newword))
				{
				case ANYCASE:
				case CAPITALIZED:
				case FOLLOWCASE:	/* Followcase can have l.c. suffix */
					return secondcap == ANYCASE;
				case ALLCAPS:
					return secondcap == ALLCAPS;
				}
			}
	    }
		else
			*p = savech;
	}
    return 0;
}

/*
 * \param word
 */
void
ISpellChecker::transposedletter (ichar_t *word)
{
    ichar_t		newword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t	temp;

    icharcpy (newword, word);
    for (p = newword;  p[1] != 0;  p++)
	{
		temp = *p;
		*p = p[1];
		p[1] = temp;
		if (good (newword, 0, 1, 0, 0))
	    {
			if (ins_cap (newword, word) < 0)
				return;
	    }
		temp = *p;
		*p = p[1];
		p[1] = temp;
	}
}

/*!
 * Insert one or more correctly capitalized versions of word
 *
 * \param word
 * \param pattern
 *
 * \return
 */
int
ISpellChecker::ins_cap (ichar_t *word, ichar_t *pattern)
{
    int			i;		/* Index into savearea */
    int			nsaved;		/* No. of words saved */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];

    nsaved = save_cap (word, pattern, savearea);
    for (i = 0;  i < nsaved;  i++)
	{
		if (insert (savearea[i]) < 0)
			return -1;
	}
    return 0;
}

/*!
 * Save one or more correctly capitalized versions of word
 *
 * \param word Word to save
 * \param pattern Prototype capitalization pattern
 * \param savearea Room to save words
 *
 * \return
 */
int
ISpellChecker::save_cap (ichar_t *word, ichar_t *pattern, 
					ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN])
{
    int			hitno;		/* Index into hits array */
    int			nsaved;		/* Number of words saved */
    int			preadd;		/* No. chars added to front of root */
    int			prestrip;	/* No. chars stripped from front */
    int			sufadd;		/* No. chars added to back of root */
    int			sufstrip;	/* No. chars stripped from back */

    if (*word == 0)
		return 0;

    for (hitno = m_numhits, nsaved = 0;  --hitno >= 0  &&  nsaved < MAX_CAPS;  )
	{
		if (m_hits[hitno].prefix)
	    {
			prestrip = m_hits[hitno].prefix->stripl;
			preadd = m_hits[hitno].prefix->affl;
	    }
		else
			prestrip = preadd = 0;
		if (m_hits[hitno].suffix)
	    {
			sufstrip = m_hits[hitno].suffix->stripl;
			sufadd = m_hits[hitno].suffix->affl;
	    }
		else
			sufadd = sufstrip = 0;
		save_root_cap (word, pattern, prestrip, preadd,
			sufstrip, sufadd,
			m_hits[hitno].dictent, m_hits[hitno].prefix, m_hits[hitno].suffix,
			savearea, &nsaved);
	}
    return nsaved;
}

/*
 * \param word
 * \param pattern
 * \param prestrip
 * \param preadd
 * \param sufstrip
 * \param sufadd
 * \param firstdent
 * \param pfxent
 * \param sufent
 *
 * \return
 */
int
ISpellChecker::ins_root_cap (ichar_t *word, ichar_t *pattern, 
				 int prestrip, int preadd, int sufstrip, int sufadd,
  				 struct dent *firstdent, struct flagent *pfxent, struct flagent *sufent)
{
    int			i;		/* Index into savearea */
    ichar_t		savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN];
    int			nsaved;		/* Number of words saved */

    nsaved = 0;
    save_root_cap (word, pattern, prestrip, preadd, sufstrip, sufadd,
      firstdent, pfxent, sufent, savearea, &nsaved);
    for (i = 0;  i < nsaved;  i++)
	{
		if (insert (savearea[i]) < 0)
			return -1;
	}
    return 0;
}

/* ARGSUSED */
/*!
 * \param word Word to be saved
 * \param pattern Capitalization pattern
 * \param prestrip No. chars stripped from front
 * \param preadd No. chars added to front of root
 * \param sufstrip No. chars stripped from back
 * \param sufadd No. chars added to back of root
 * \param firstdent First dent for root
 * \param pfxent Pfx-flag entry for word
 * \param sufent Sfx-flag entry for word
 * \param savearea Room to save words
 * \param nsaved Number saved so far (updated)
 */
void
ISpellChecker::save_root_cap (ichar_t *word, ichar_t *pattern, 
						  int prestrip, int preadd, int sufstrip, int sufadd,
						  struct dent *firstdent, struct flagent *pfxent, struct flagent *sufent, 
						  ichar_t savearea[MAX_CAPS][INPUTWORDLEN + MAXAFFIXLEN], 
					      int * nsaved)
{
#ifndef NO_CAPITALIZATION_SUPPORT
    register struct dent * dent;
#endif /* NO_CAPITALIZATION_SUPPORT */
    int			firstisupper;
    ichar_t		newword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];
#ifndef NO_CAPITALIZATION_SUPPORT
    register ichar_t *	p;
    int			len;
    int			i;
    int			limit;
#endif /* NO_CAPITALIZATION_SUPPORT */

    if (*nsaved >= MAX_CAPS)
		return;
    icharcpy (newword, word);
    firstisupper = myupper (pattern[0]);
#ifdef NO_CAPITALIZATION_SUPPORT
    /*
    ** Apply the old, simple-minded capitalization rules.
    */
    if (firstisupper)
	{
		if (myupper (pattern[1]))
			upcase (newword);
		else
	    {
			lowcase (newword);
			newword[0] = mytoupper (newword[0]);
	    }
	}
    else
		lowcase (newword);
    icharcpy (savearea[*nsaved], newword);
    (*nsaved)++;
    return;
#else /* NO_CAPITALIZATION_SUPPORT */
#define flagsareok(dent)    \
    ((pfxent == NULL \
	||  TSTMASKBIT (dent->mask, pfxent->flagbit)) \
      &&  (sufent == NULL \
	||  TSTMASKBIT (dent->mask, sufent->flagbit)))

    dent = firstdent;
    if ((dent->flagfield & (CAPTYPEMASK | MOREVARIANTS)) == ALLCAPS)
	{
		upcase (newword);	/* Uppercase required */
		icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
	}
    for (p = pattern;  *p;  p++)
	{
		if (mylower (*p))
			break;
	}
    if (*p == 0)
	{
		upcase (newword);	/* Pattern was all caps */
		icharcpy (savearea[*nsaved], newword);
		(*nsaved)++;
		return;
	}
    for (p = pattern + 1;  *p;  p++)
	{
		if (myupper (*p))
			break;
	}
    if (*p == 0)
	{
		/*
		** The pattern was all-lower or capitalized.  If that's
		** legal, insert only that version.
		*/
		if (firstisupper)
		{
			if (captype (dent->flagfield) == CAPITALIZED
			  ||  captype (dent->flagfield) == ANYCASE)
			{
				lowcase (newword);
				newword[0] = mytoupper (newword[0]);
				icharcpy (savearea[*nsaved], newword);
				(*nsaved)++;
				return;
			}
		}
		else
		{
			if (captype (dent->flagfield) == ANYCASE)
			{
				lowcase (newword);
				icharcpy (savearea[*nsaved], newword);
				(*nsaved)++;
				return;
			}
		}
		while (dent->flagfield & MOREVARIANTS)
		{
			dent = dent->next;
			if (captype (dent->flagfield) == FOLLOWCASE
			  ||  !flagsareok (dent))
				continue;
			if (firstisupper)
			{
				if (captype (dent->flagfield) == CAPITALIZED)
				{
					lowcase (newword);
					newword[0] = mytoupper (newword[0]);
					icharcpy (savearea[*nsaved], newword);
					(*nsaved)++;
					return;
				}
			}
			else
			{
				if (captype (dent->flagfield) == ANYCASE)
				{
					lowcase (newword);
					icharcpy (savearea[*nsaved], newword);
					(*nsaved)++;
					return;
				}
			}
	    }
	}
    /*
    ** Either the sample had complex capitalization, or the simple
    ** capitalizations (all-lower or capitalized) are illegal.
    ** Insert all legal capitalizations, including those that are
    ** all-lower or capitalized.  If the prototype is capitalized,
    ** capitalized all-lower samples.  Watch out for affixes.
    */
    dent = firstdent;
    p = strtosichar (dent->word, 1);
    len = icharlen (p);
    if (dent->flagfield & MOREVARIANTS)
		dent = dent->next;	/* Skip place-holder entry */
    for (  ;  ;  )
	{
		if (flagsareok (dent))
	    {
			if (captype (dent->flagfield) != FOLLOWCASE)
			{
				lowcase (newword);
				if (firstisupper  ||  captype (dent->flagfield) == CAPITALIZED)
					newword[0] = mytoupper (newword[0]);
				icharcpy (savearea[*nsaved], newword);
				(*nsaved)++;
				if (*nsaved >= MAX_CAPS)
					return;
			}
			else
			{
				/* Followcase is the tough one. */
				p = strtosichar (dent->word, 1);
				memmove (
				  reinterpret_cast<char *>(newword + preadd),
				  reinterpret_cast<char *>(p + prestrip),
				  (len - prestrip - sufstrip) * sizeof (ichar_t));
				if (myupper (p[prestrip]))
				{
					for (i = 0;  i < preadd;  i++)
						newword[i] = mytoupper (newword[i]);
				}
				else
				{
					for (i = 0;  i < preadd;  i++)
						newword[i] = mytolower (newword[i]);
				}
				limit = len + preadd + sufadd - prestrip - sufstrip;
				i = len + preadd - prestrip - sufstrip;
				p += len - sufstrip - 1;
				if (myupper (*p))
				{
					for (p = newword + i;  i < limit;  i++, p++)
						*p = mytoupper (*p);
				}
				else
				{
					for (p = newword + i;  i < limit;  i++, p++)
						*p = mytolower (*p);
				}
				icharcpy (savearea[*nsaved], newword);
				(*nsaved)++;
				if (*nsaved >= MAX_CAPS)
					return;
			}
	    }
		if ((dent->flagfield & MOREVARIANTS) == 0)
			break;		/* End of the line */
		dent = dent->next;
	}
    return;
#endif /* NO_CAPITALIZATION_SUPPORT */
}


