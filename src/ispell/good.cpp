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
 * good.c - see if a word or its root word
 * is in the dictionary.
 *
 * Pace Willisson, 1983
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
 * Revision 1.2  2003/07/16 22:52:37  dom
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
 * Revision 1.1  2003/01/24 05:52:32  hippietrail
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
 * Revision 1.6  2003/01/06 18:48:38  dom
 * ispell cleanup, start of using new 'add' save features
 *
 * Revision 1.5  2002/09/19 05:31:15  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.4  2002/09/17 03:03:29  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.3  2002/09/13 17:20:12  mpritchett
 * Fix more warnings for Linux build
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
 * Revision 1.5  2000/02/09 22:35:25  sterwill
 * Clean up some warnings
 *
 * Revision 1.4  1998/12/29 14:55:32  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.3  1998/12/28 23:11:30  eric
 *
 * modified spell code and integration to build on Windows.
 * This is still a hack.
 *
 * Actually, it doesn't yet WORK on Windows.  It just builds.
 * SpellCheckInit is failing for some reason.
 *
 * Revision 1.2  1998/12/28 22:16:22  eric
 *
 * These changes begin to incorporate the spell checker into AbiWord.  Most
 * of this is a hack.
 *
 * 1.  added other/spell to the -I list in config/abi_defs
 * 2.  replaced other/spell/Makefile with one which is more like
 * 	our build system.
 * 3.  added other/spell to other/Makefile so that the build will now
 * 	dive down and build the spell check library.
 * 4.  added the AbiSpell library to the Makefiles in wp/main
 * 5.  added a call to SpellCheckInit in wp/main/unix/UnixMain.cpp.
 * 	This call is a HACK and should be replaced with something
 * 	proper later.
 * 6.  added code to fv_View.cpp as follows:
 * 	whenever you double-click on a word, the spell checker
 * 	verifies that word and prints its status to stdout.
 *
 * Caveats:
 * 1.  This will break the Windows build.  I'm going to work on fixing it
 * 	now.
 * 2.  This only works if your dictionary is in /usr/lib/ispell/american.hash.
 * 	The dictionary location is currently hard-coded.  This will be
 * 	fixed as well.
 *
 * Anyway, such as it is, it works.
 *
 * Revision 1.1  1998/12/28 18:04:43  davet
 * Spell checker code stripped from ispell.  At this point, there are
 * two external routines...  the Init routine, and a check-a-word routine
 * which returns a boolean value, and takes a 16 bit char string.
 * The code resembles the ispell code as much as possible still.
 *
 * Revision 1.43  1994/11/02  06:56:05  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.42  1994/10/25  05:45:59  geoff
 * Add support for an affix that will work with any word, even if there's
 * no explicit flag.
 *
 * Revision 1.41  1994/05/24  06:23:06  geoff
 * Let tgood decide capitalization questions, rather than doing it ourselves.
 *
 * Revision 1.40  1994/05/17  06:44:10  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.39  1994/01/25  07:11:31  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ispell_checker.h"


int		good P ((ichar_t * word, int ignoreflagbits, int allhits,
			 int pfxopts, int sfxopts));

#ifndef NO_CAPITALIZATION_SUPPORT

/*!
** See if this particular capitalization (dent) is legal with these
** particular affixes.
**
** \param dent
** \param hit
**
** \return
*/
static int entryhasaffixes (struct dent *dent, struct success *hit)
{
    if (hit->prefix  &&  !TSTMASKBIT (dent->mask, hit->prefix->flagbit))
		return 0;
    if (hit->suffix  &&  !TSTMASKBIT (dent->mask, hit->suffix->flagbit))
		return 0;
    return 1;			/* Yes, these affixes are legal */
}

/*
 * \param word
 * \param hit
 * \param len
 *
 * \return
 */
int ISpellChecker::cap_ok (ichar_t *word, struct success *hit, int len)
{
    register ichar_t *		dword;
    register ichar_t *		w;
    register struct dent *	dent;
    ichar_t			dentword[INPUTWORDLEN + MAXAFFIXLEN];
    int				preadd;
    int				prestrip;
    int				sufadd;
    ichar_t *		limit;
    long			thiscap;
    long			dentcap;

    thiscap = whatcap (word);
    /*
    ** All caps is always legal, regardless of affixes.
    */
    preadd = prestrip = sufadd = 0;
    if (thiscap == ALLCAPS)
		return 1;
    else if (thiscap == FOLLOWCASE)
	{
		/* Set up some constants for the while(1) loop below */
		if (hit->prefix)
		{
			preadd = hit->prefix->affl;
			prestrip = hit->prefix->stripl;
		}
		else
			preadd = prestrip = 0;
		sufadd = hit->suffix ? hit->suffix->affl : 0;
	}
    /*
    ** Search the variants for one that matches what we have.  Note
    ** that thiscap can't be ALLCAPS, since we already returned
    ** for that case.
    */
    dent = hit->dictent;
    for (  ;  ;  )
	{
		dentcap = captype (dent->flagfield);
		if (dentcap != thiscap)
		{
			if (dentcap == ANYCASE  &&  thiscap == CAPITALIZED
			 &&  entryhasaffixes (dent, hit))
				return 1;
		}
		else				/* captypes match */
		{
			if (thiscap != FOLLOWCASE)
			{
				if (entryhasaffixes (dent, hit))
					return 1;
			}
			else
			{
				/*
				** Make sure followcase matches exactly.
				** Life is made more difficult by the
				** possibility of affixes.  Start with
				** the prefix.
				*/
				strtoichar (dentword, dent->word, INPUTWORDLEN, 1);
				dword = dentword;
				limit = word + preadd;
				if (myupper (dword[prestrip]))
				{
					for (w = word;  w < limit;  w++)
					{
						if (mylower (*w))
							goto doublecontinue;
					}
				}
				else
				{
					for (w = word;  w < limit;  w++)
					{
						if (myupper (*w))
							goto doublecontinue;
					}
				}
				dword += prestrip;
				/* Do root part of word */
				limit = dword + len - preadd - sufadd;
				while (dword < limit)
				{
					if (*dword++ != *w++)
						goto doublecontinue;
				}
				/* Do suffix */
				dword = limit - 1;
				if (myupper (*dword))
				{
					for (  ;  *w;  w++)
					{
						if (mylower (*w))
							goto doublecontinue;
					}
				}
				else
				{
					for (  ;  *w;  w++)
					{
						if (myupper (*w))
							goto doublecontinue;
					}
				}
				/*
				** All failure paths go to "doublecontinue,"
				** so if we get here it must match.
				*/
				if (entryhasaffixes (dent, hit))
					return 1;
				doublecontinue:	;
			}
		}
		if ((dent->flagfield & MOREVARIANTS) == 0)
			break;
		dent = dent->next;
	}

    /* No matches found */
    return 0;
}
#endif

#ifndef NO_CAPITALIZATION_SUPPORT
/*!
 * \param w Word to look up
 * \param ignoreflagbits NZ to ignore affix flags in dict
 * \param allhits NZ to ignore case, get every hit
 * \param pfxopts Options to apply to prefixes
 * \param sfxopts Options to apply to suffixes
 *
 * \return
 */
int ISpellChecker::good (ichar_t *w, int ignoreflagbits, int allhits, int pfxopts, int sfxopts)
#else
/* ARGSUSED */
int ISpellChecker::good (ichar_t *w, int ignoreflagbits, int dummy, int pfxopts, int sfxopts)
#endif
{
    ichar_t		nword[INPUTWORDLEN + MAXAFFIXLEN];
    register ichar_t *	p;
    register ichar_t *	q;
    register int	n;
    register struct dent * dp;

    /*
    ** Make an uppercase copy of the word we are checking.
    */
    for (p = w, q = nword;  *p;  )
		*q++ = mytoupper (*p++);
    *q = 0;
    n = q - nword;

    m_numhits = 0;

    if ((dp = ispell_lookup (nword, 1)) != NULL)
	{
		m_hits[0].dictent = dp;
		m_hits[0].prefix = NULL;
		m_hits[0].suffix = NULL;
#ifndef NO_CAPITALIZATION_SUPPORT
		if (allhits  ||  cap_ok (w, &m_hits[0], n))
			m_numhits = 1;
#else
		m_numhits = 1;
#endif
	}

    if (m_numhits  &&  !allhits)
		return 1;

    /* try stripping off affixes */

    chk_aff (w, nword, n, ignoreflagbits, allhits, pfxopts, sfxopts);

    return m_numhits;
}




