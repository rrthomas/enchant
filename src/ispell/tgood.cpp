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
 * Copyright 1987, 1988, 1989, 1992, 1993, Geoff Kuenning, Granada Hills, CA
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
 * Table-driven version of good.c.
 *
 * Geoff Kuenning, July 1987
 */

/*
 * $Log$
 * Revision 1.4  2003/08/14 17:51:29  dom
 * update license - exception clause should be Lesser GPL
 *
 * Revision 1.3  2003/07/28 20:40:28  dom
 * fix up the license clause, further win32-registry proof some directory getting functions
 *
 * Revision 1.2  2003/07/16 22:52:56  dom
 * LGPL + exception license
 *
 * Revision 1.1  2003/07/15 01:15:09  dom
 * ispell enchant backend
 *
 * Revision 1.2  2003/01/29 05:50:12  hippietrail
 *
 * Fixed my mess in EncodingManager.
 * Changed many C casts to C++ casts.
 *
 * Revision 1.1  2003/01/24 05:52:36  hippietrail
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
 * Revision 1.6  2003/01/06 18:48:42  dom
 * ispell cleanup, start of using new 'add' save features
 *
 * Revision 1.5  2002/09/19 05:31:20  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.4  2002/09/17 03:03:31  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.3  2002/09/13 17:20:14  mpritchett
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
 * Revision 1.7  1999/10/20 06:03:56  sterwill
 * Changed C++-style comments to C-style comments in C code.
 *
 * Revision 1.6  1999/10/20 03:19:35  paul
 * Hacked ispell code to ignore any characters that don't fit in the lookup tables loaded from the dictionary.  It ain't pretty, but at least we don't crash there any more.
 *
 * Revision 1.5  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.4  1998/12/29 14:55:33  eric
 *
 * I've doctored the ispell code pretty extensively here.  It is now
 * warning-free on Win32.  It also *works* on Win32 now, since I
 * replaced all the I/O calls with ANSI standard ones.
 *
 * Revision 1.4  1998/12/29 14:55:33  eric
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
 * Revision 1.32  1994/11/02  06:56:16  geoff
 * Remove the anyword feature, which I've decided is a bad idea.
 *
 * Revision 1.31  1994/10/25  05:46:25  geoff
 * Add support for the FF_ANYWORD (affix applies to all words, even if
 * flag bit isn't set) flag option.
 *
 * Revision 1.30  1994/05/24  06:23:08  geoff
 * Don't create a hit if "allhits" is clear and capitalization
 * mismatches.  This cures a bug where a word could be in the dictionary
 * and yet not found.
 *
 * Revision 1.29  1994/05/17  06:44:21  geoff
 * Add support for controlled compound formation and the COMPOUNDONLY
 * option to affix flags.
 *
 * Revision 1.28  1994/01/25  07:12:13  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ispell_checker.h"

/*!
 * Check possible affixes
 *
 * \param word Word to be checked
 * \param ucword Upper-case-only copy of word
 * \param len The length of word/ucword
 * \param ignoreflagbits Ignore whether affix is legal
 * \param allhits Keep going after first hit
 * \param pfxopts Options to apply to prefixes
 * \param sfxopts Options to apply to suffixes
 */
void ISpellChecker::chk_aff (ichar_t *word, ichar_t *ucword, 
			  int len, int ignoreflagbits, int allhits, int pfxopts, int sfxopts)
{
    register ichar_t *	cp;		/* Pointer to char to index on */
    struct flagptr *	ind;		/* Flag index table to test */

    pfx_list_chk (word, ucword, len, pfxopts, sfxopts, &m_pflagindex[0],
      ignoreflagbits, allhits);
    cp = ucword;
	/* HACK: bail on unrecognized chars */
	if (*cp >= (SET_SIZE + MAXSTRINGCHARS))
		return;
    ind = &m_pflagindex[*cp++];
    while (ind->numents == 0  &&  ind->pu.fp != NULL)
	{
		if (*cp == 0)
			return;
		if (ind->pu.fp[0].numents)
		{
			pfx_list_chk (word, ucword, len, pfxopts, sfxopts, &ind->pu.fp[0],
			  ignoreflagbits, allhits);
			if (m_numhits  &&  !allhits  &&  /* !cflag  && */  !ignoreflagbits)
				return;
		}
		/* HACK: bail on unrecognized chars */
		if (*cp >= (SET_SIZE + MAXSTRINGCHARS))
			return;
		ind = &ind->pu.fp[*cp++];
	}
    pfx_list_chk (word, ucword, len, pfxopts, sfxopts, ind, ignoreflagbits,
      allhits);
    if (m_numhits  &&  !allhits  &&  /* !cflag  &&*/  !ignoreflagbits)
		return;
    chk_suf (word, ucword, len, sfxopts, static_cast<struct flagent *>(NULL),
      ignoreflagbits, allhits);
}

/*!
 * Check some prefix flags
 *
 * \param word Word to be checked
 * \param ucword Upper-case-only word
 * \param len The length of ucword
 * \param optflags Options to apply
 * \param sfxopts Options to apply to suffixes
 * \param ind Flag index table
 * \param ignoreflagbits Ignore whether affix is legal
 * \param allhits Keep going after first hit
 * */
void ISpellChecker::pfx_list_chk (ichar_t *word, ichar_t *ucword, int len, int optflags, 
					int sfxopts, struct flagptr * ind, int ignoreflagbits, int allhits)
{
    int			cond;		/* Condition number */
    register ichar_t *	cp;		/* Pointer into end of ucword */
    struct dent *	dent;		/* Dictionary entry we found */
    int			entcount;	/* Number of entries to process */
    register struct flagent *
			flent;		/* Current table entry */
    int			preadd;		/* Length added to tword2 as prefix */
    register int	tlen;		/* Length of tword */
    ichar_t		tword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4]; /* Tmp cpy */
    ichar_t		tword2[sizeof tword]; /* 2nd copy for ins_root_cap */

    for (flent = ind->pu.ent, entcount = ind->numents;
      entcount > 0;
      flent++, entcount--)
	{
		/*
		 * If this is a compound-only affix, ignore it unless we're
		 * looking for that specific thing.
		 */
		if ((flent->flagflags & FF_COMPOUNDONLY) != 0
		  &&  (optflags & FF_COMPOUNDONLY) == 0)
			continue;

		/*
		 * See if the prefix matches.
		 */
		tlen = len - flent->affl;
		if (tlen > 0
		  &&  (flent->affl == 0
			||  icharncmp (flent->affix, ucword, flent->affl) == 0)
		  &&  tlen + flent->stripl >= flent->numconds)
		{
			/*
			 * The prefix matches.  Remove it, replace it by the "strip"
			 * string (if any), and check the original conditions.
			 */
			if (flent->stripl)
				icharcpy (tword, flent->strip);
			icharcpy (tword + flent->stripl, ucword + flent->affl);
			cp = tword;
			for (cond = 0;  cond < flent->numconds;  cond++)
			{
				if ((flent->conds[*cp++] & (1 << cond)) == 0)
					break;
			}
			if (cond >= flent->numconds)
			{
				/*
				 * The conditions match.  See if the word is in the
				 * dictionary.
				 */
				tlen += flent->stripl;

				if (ignoreflagbits)
				{
					if ((dent = ispell_lookup (tword, 1)) != NULL)
					{
						cp = tword2;
						if (flent->affl)
						{
							icharcpy (cp, flent->affix);
							cp += flent->affl;
							*cp++ = '+';
						}
						preadd = cp - tword2;
						icharcpy (cp, tword);
						cp += tlen;
						if (flent->stripl)
						{
							*cp++ = '-';
							icharcpy (cp, flent->strip);
						}
					}
				}
				else if ((dent = ispell_lookup (tword, 1)) != NULL
				  &&  TSTMASKBIT (dent->mask, flent->flagbit))
				{
					if (m_numhits < MAX_HITS)
					{
						m_hits[m_numhits].dictent = dent;
						m_hits[m_numhits].prefix = flent;
						m_hits[m_numhits].suffix = NULL;
						m_numhits++;
					}
					if (!allhits)
					{
#ifndef NO_CAPITALIZATION_SUPPORT
						if (cap_ok (word, &m_hits[0], len))
							return;
						m_numhits = 0;
#else /* NO_CAPITALIZATION_SUPPORT */
						return;
#endif /* NO_CAPITALIZATION_SUPPORT */
					}
				}
				/*
				 * Handle cross-products.
				 */
				if (flent->flagflags & FF_CROSSPRODUCT)
						chk_suf (word, tword, tlen, sfxopts | FF_CROSSPRODUCT,
					flent, ignoreflagbits, allhits);
			}
	    }
	}
}

/*!
 * Check possible suffixes
 *
 * \param word Word to be checked
 * \param ucword Upper-case-only word
 * \param len The length of ucword
 * \param optflags Affix option flags
 * \param pfxent Prefix flag entry if cross-prod
 * \param ignoreflagbits Ignore whether affix is legal
 * \param allhits Keep going after first hit
 */
void
ISpellChecker::chk_suf (ichar_t *word, ichar_t *ucword, 
					int len, int optflags, struct flagent *pfxent, 
					int ignoreflagbits, int allhits)
{
    register ichar_t *	cp;		/* Pointer to char to index on */
    struct flagptr *	ind;		/* Flag index table to test */

    suf_list_chk (word, ucword, len, &m_sflagindex[0], optflags, pfxent,
      ignoreflagbits, allhits);
    cp = ucword + len - 1;
	/* HACK: bail on unrecognized chars */
	if (*cp >= (SET_SIZE + MAXSTRINGCHARS))
		return;
    ind = &m_sflagindex[*cp];
    while (ind->numents == 0  &&  ind->pu.fp != NULL)
	{
		if (cp == ucword)
			return;
		if (ind->pu.fp[0].numents)
		{
			suf_list_chk (word, ucword, len, &ind->pu.fp[0],
			  optflags, pfxent, ignoreflagbits, allhits);
			if (m_numhits != 0  &&  !allhits  &&  /* !cflag  && */  !ignoreflagbits)
				return;
		}
		/* HACK: bail on unrecognized chars */
		if (*(cp-1) >= (SET_SIZE + MAXSTRINGCHARS))
			return;
		ind = &ind->pu.fp[*--cp];
	}
    suf_list_chk (word, ucword, len, ind, optflags, pfxent,
      ignoreflagbits, allhits);
}
    
/*!
 * \param word Word to be checked
 * \param ucword Upper-case-only word
 * \param len The length of ucword
 * \param ind Flag index table
 * \param optflags Affix option flags
 * \param pfxent Prefix flag entry if crossonly
 * \param ignoreflagbits Ignore whether affix is legal
 * \pram allhits Keep going after first hit
 */
void ISpellChecker::suf_list_chk (ichar_t *word, ichar_t *ucword, 
						  int len, struct flagptr *ind, int optflags, 
						  struct flagent *pfxent, int ignoreflagbits, int allhits)
{
    register ichar_t *	cp;		/* Pointer into end of ucword */
    int			cond;		/* Condition number */
    struct dent *	dent;		/* Dictionary entry we found */
    int			entcount;	/* Number of entries to process */
    register struct flagent *
			flent;		/* Current table entry */
    int			preadd;		/* Length added to tword2 as prefix */
    register int	tlen;		/* Length of tword */
    ichar_t		tword[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4]; /* Tmp cpy */
    ichar_t		tword2[sizeof tword]; /* 2nd copy for ins_root_cap */

    icharcpy (tword, ucword);
    for (flent = ind->pu.ent, entcount = ind->numents;
      entcount > 0;
      flent++, entcount--)
	{
		if ((optflags & FF_CROSSPRODUCT) != 0
		  &&  (flent->flagflags & FF_CROSSPRODUCT) == 0)
			continue;
		/*
		 * If this is a compound-only affix, ignore it unless we're
		 * looking for that specific thing.
		 */
		if ((flent->flagflags & FF_COMPOUNDONLY) != 0
		  &&  (optflags & FF_COMPOUNDONLY) == 0)
			continue;

		/*
		 * See if the suffix matches.
		 */
		tlen = len - flent->affl;
		if (tlen > 0
		  &&  (flent->affl == 0
			||  icharcmp (flent->affix, ucword + tlen) == 0)
		  &&  tlen + flent->stripl >= flent->numconds)
		{
			/*
			 * The suffix matches.  Remove it, replace it by the "strip"
			 * string (if any), and check the original conditions.
			 */
			icharcpy (tword, ucword);
			cp = tword + tlen;
			if (flent->stripl)
			{
				icharcpy (cp, flent->strip);
				tlen += flent->stripl;
				cp = tword + tlen;
			}
			else
				*cp = '\0';
			for (cond = flent->numconds;  --cond >= 0;  )
			{
				if ((flent->conds[*--cp] & (1 << cond)) == 0)
					break;
			}
			if (cond < 0)
			{
				/*
				 * The conditions match.  See if the word is in the
				 * dictionary.
				 */
				if (ignoreflagbits)
				{
					if ((dent = ispell_lookup (tword, 1)) != NULL)
					{
						cp = tword2;
						if ((optflags & FF_CROSSPRODUCT)
						  &&  pfxent->affl != 0)
						{
							icharcpy (cp, pfxent->affix);
							cp += pfxent->affl;
							*cp++ = '+';
						}
						preadd = cp - tword2;
						icharcpy (cp, tword);
						cp += tlen;
						if ((optflags & FF_CROSSPRODUCT)
						  &&  pfxent->stripl != 0)
						{
							*cp++ = '-';
							icharcpy (cp, pfxent->strip);
							cp += pfxent->stripl;
						}
						if (flent->stripl)
						{
							*cp++ = '-';
							icharcpy (cp, flent->strip);
							cp += flent->stripl;
						}
						if (flent->affl)
						{
							*cp++ = '+';
							icharcpy (cp, flent->affix);
							cp += flent->affl;
						}
					}
				}
				else if ((dent = ispell_lookup (tword, 1)) != NULL
				  &&  TSTMASKBIT (dent->mask, flent->flagbit)
				  &&  ((optflags & FF_CROSSPRODUCT) == 0
					|| TSTMASKBIT (dent->mask, pfxent->flagbit)))
				{
					if (m_numhits < MAX_HITS)
					{
						m_hits[m_numhits].dictent = dent;
						m_hits[m_numhits].prefix = pfxent;
						m_hits[m_numhits].suffix = flent;
						m_numhits++;
					}
					if (!allhits)
					{
#ifndef NO_CAPITALIZATION_SUPPORT
						if (cap_ok (word, &m_hits[0], len))
							return;
						m_numhits = 0;
#else /* NO_CAPITALIZATION_SUPPORT */
						return;
#endif /* NO_CAPITALIZATION_SUPPORT */
					}
				}
			}
		}
	}
}

/*!
 * Expand a dictionary prefix entry
 *
 * \param croot Char version of rootword
 * \param rootword Root word to expand
 * \param mask Mask bits to expand on
 * \param option Option, see expandmode
 * \param extra Extra info to add to line
 *
 * \return
 */
int ISpellChecker::expand_pre (char *croot, ichar_t *rootword, MASKTYPE mask[], 
				int option, char *extra)
{
    int				entcount;	/* No. of entries to process */
    int				explength;	/* Length of expansions */
    register struct flagent *
				flent;		/* Current table entry */

    for (flent = m_pflaglist, entcount = m_numpflags, explength = 0;
      entcount > 0;
      flent++, entcount--)
	{
		if (TSTMASKBIT (mask, flent->flagbit))
			explength +=
			  pr_pre_expansion (croot, rootword, flent, mask, option, extra);
	}
    return explength;
}

/*!
 * Print a prefix expansion
 *
 * \param croot Char version of rootword
 * \param rootword Root word to expand
 * \param flent Current table entry
 * \param mask Mask bits to expand on
 * \param option Option, see	expandmode
 * \param extra Extra info to add to line
 *
 * \return
 */
int ISpellChecker::pr_pre_expansion ( char *croot, ichar_t *rootword, 
							struct flagent *flent, MASKTYPE mask[], int option, 
							char *extra)
{
    int				cond;		/* Current condition number */
    register ichar_t *		nextc;		/* Next case choice */
    int				tlen;		/* Length of tword */
    ichar_t			tword[INPUTWORDLEN + MAXAFFIXLEN]; /* Temp */

    tlen = icharlen (rootword);
    if (flent->numconds > tlen)
		return 0;
    tlen -= flent->stripl;
    if (tlen <= 0)
		return 0;
    tlen += flent->affl;
    for (cond = 0, nextc = rootword;  cond < flent->numconds;  cond++)
	{
		if ((flent->conds[mytoupper (*nextc++)] & (1 << cond)) == 0)
			return 0;
	}
    /*
     * The conditions are satisfied.  Copy the word, add the prefix,
     * and make it the proper case.   This code is carefully written
     * to match that ins_cap and cap_ok.  Note that the affix, as
     * inserted, is uppercase.
     *
     * There is a tricky bit here:  if the root is capitalized, we
     * want a capitalized result.  If the root is followcase, however,
     * we want to duplicate the case of the first remaining letter
     * of the root.  In other words, "Loved/U" should generate "Unloved",
     * but "LOved/U" should generate "UNLOved" and "lOved/U" should
     * produce "unlOved".
     */
    if (flent->affl)
	{
		icharcpy (tword, flent->affix);
		nextc = tword + flent->affl;
	}
    icharcpy (nextc, rootword + flent->stripl);
    if (myupper (rootword[0]))
	{
		/* We must distinguish followcase from capitalized and all-upper */
		for (nextc = rootword + 1;  *nextc;  nextc++)
		{
			if (!myupper (*nextc))
				break;
		}
		if (*nextc)
		{
			/* It's a followcase or capitalized word.  Figure out which. */
			for (  ;  *nextc;  nextc++)
			{
				if (myupper (*nextc))
					break;
			}
			if (*nextc)
			{
				/* It's followcase. */
				if (!myupper (tword[flent->affl]))
					forcelc (tword, flent->affl);
			}
			else
			{
				/* It's capitalized */
				forcelc (tword + 1, tlen - 1);
			}
		}
	}
    else
	{
		/* Followcase or all-lower, we don't care which */
		if (!myupper (*nextc))
			forcelc (tword, flent->affl);
	}
    if (option == 3)
		printf ("\n%s", croot);
    if (option != 4)
		printf (" %s%s", ichartosstr (tword, 1), extra);
    if (flent->flagflags & FF_CROSSPRODUCT)
		return tlen
		  + expand_suf (croot, tword, mask, FF_CROSSPRODUCT, option, extra);
    else
		return tlen;
}

/*!
 * Expand a dictionary suffix entry
 *
 * \param croot Char version of rootword
 * \param rootword Root word to expand 
 * \param mask Mask bits to expand on
 * \param optflags Affix option flags
 * \param option Option, see expandmode
 * \param extra Extra info to add to line
 *
 * \return
 */
int ISpellChecker::expand_suf (char *croot, ichar_t *rootword, MASKTYPE mask[], 
				int optflags, int option, char *extra)
{
    int				entcount;	/* No. of entries to process */
    int				explength;	/* Length of expansions */
    register struct flagent *
				flent;		/* Current table entry */

    for (flent = m_sflaglist, entcount = m_numsflags, explength = 0;
      entcount > 0;
      flent++, entcount--)
	{
		if (TSTMASKBIT (mask, flent->flagbit))
		{
			if ((optflags & FF_CROSSPRODUCT) == 0
			  ||  (flent->flagflags & FF_CROSSPRODUCT))
			explength +=
			  pr_suf_expansion (croot, rootword, flent, option, extra);
		}
	}
    return explength;
}

/*!
 * Print a suffix expansion
 *
 * \param croot Char version of rootword
 * \param rootword Root word to expand
 * \param flent Current table entry
 * \param option Option, see expandmode
 * \param extra Extra info to add to line
 *
 * \return
 */
int ISpellChecker::pr_suf_expansion (char *croot, ichar_t *rootword, 
							struct flagent *flent, int option, char *extra)
{
    int				cond;		/* Current condition number */
    register ichar_t *		nextc;		/* Next case choice */
    int				tlen;		/* Length of tword */
    ichar_t			tword[INPUTWORDLEN + MAXAFFIXLEN]; /* Temp */

    tlen = icharlen (rootword);
    cond = flent->numconds;
    if (cond > tlen)
		return 0;
    if (tlen - flent->stripl <= 0)
		return 0;
    for (nextc = rootword + tlen;  --cond >= 0;  )
	{
		if ((flent->conds[mytoupper (*--nextc)] & (1 << cond)) == 0)
			return 0;
	}
    /*
     * The conditions are satisfied.  Copy the word, add the suffix,
     * and make it match the case of the last remaining character of the
     * root.  Again, this code carefully matches ins_cap and cap_ok.
     */
    icharcpy (tword, rootword);
    nextc = tword + tlen - flent->stripl;
    if (flent->affl)
	{
		icharcpy (nextc, flent->affix);
		if (!myupper (nextc[-1]))
			forcelc (nextc, flent->affl);
	}
    else
		*nextc = 0;
    if (option == 3)
		printf ("\n%s", croot);
    if (option != 4)
		printf (" %s%s", ichartosstr (tword, 1), extra);
    return tlen + flent->affl - flent->stripl;
}

/*!
 * \param dst Destination to modify
 * \param len Length to copy
 */
void ISpellChecker::forcelc (ichar_t *dst, int len)			/* Force to lowercase */
{

    for (  ;  --len >= 0;  dst++)
		*dst = mytolower (*dst);
}
