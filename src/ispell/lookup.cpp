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
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * lookup.c - see if a word appears in the dictionary
 *
 * Pace Willisson, 1983
 *
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
 * $Log$
 * Revision 1.7  2003/09/25 02:44:48  dom
 * bug 5813
 *
 * Revision 1.6  2003/08/26 13:20:40  dom
 * ispell crasher fix, implement enchant_dictionary_release
 *
 * Revision 1.5  2003/08/26 13:08:03  uwog
 * Fix segfault when the requested dictionary couldn't be found.
 *
 * Revision 1.4  2003/08/14 16:27:36  dom
 * update some documentation
 *
 * Revision 1.3  2003/07/28 20:40:27  dom
 * fix up the license clause, further win32-registry proof some directory getting functions
 *
 * Revision 1.2  2003/07/16 22:52:47  dom
 * LGPL + exception license
 *
 * Revision 1.1  2003/07/15 01:15:07  dom
 * ispell enchant backend
 *
 * Revision 1.3  2003/01/29 05:50:12  hippietrail
 *
 * Fixed my mess in EncodingManager.
 * Changed many C casts to C++ casts.
 *
 * Revision 1.2  2003/01/25 03:16:05  hippietrail
 *
 * An UT_ICONV_INVALID fix which escaped the last commit.
 *
 * Revision 1.1  2003/01/24 05:52:34  hippietrail
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
 * Revision 1.12  2003/01/06 18:48:39  dom
 * ispell cleanup, start of using new 'add' save features
 *
 * Revision 1.11  2002/09/19 05:31:17  hippietrail
 *
 * More Ispell cleanup.  Conditional globals and DEREF macros are removed.
 * K&R function declarations removed, converted to Doxygen style comments
 * where possible.  No code has been changed (I hope).  Compiles for me but
 * unable to test.
 *
 * Revision 1.10  2002/09/17 03:03:30  hippietrail
 *
 * After seeking permission on the developer list I've reformatted all the
 * spelling source which seemed to have parts which used 2, 3, 4, and 8
 * spaces for tabs.  It should all look good with our standard 4-space
 * tabs now.
 * I've concentrated just on indentation in the actual code.  More prettying
 * could be done.
 * * NO code changes were made *
 *
 * Revision 1.9  2002/09/13 17:20:13  mpritchett
 * Fix more warnings for Linux build
 *
 * Revision 1.8  2002/05/03 09:49:43  fjfranklin
 * o hash downloader update (Gabriel Gerhardsson)
 * - Comment out the "Can't open <dictionary>" printf.
 * - Make the progressbar more clean at the begining of the download.
 * - Add support for tarballs that doesn't have the full path included
 * - Fix copyright headers on the newly added files (*HashDownloader.*)
 *
 * Revision 1.7  2001/08/27 19:06:30  dom
 * Lots of compilation fixes
 *
 * Revision 1.6  2001/08/10 18:32:40  dom
 * Spelling and iconv updates. god, i hate iconv
 *
 * Revision 1.5  2001/08/10 09:57:49  hub
 * Patch by sobomax@FreeBSD.org
 * #include "iconv.h" directive is missed from src/other/spell/xp/lookup.c and
 * src/wp/impexp/xp/ie_imp_RTF.cpp.
 * See bug 1823
 *
 * Revision 1.4  2001/07/18 17:46:01  dom
 * Module changes, and fix compiler warnings
 *
 * Revision 1.3  2001/06/12 21:32:49  dom
 * More ispell work...
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
 * Revision 1.7  1999/09/29 23:33:32  justin
 * Updates to the underlying ispell-based code to support suggested corrections.
 *
 * Revision 1.6  1999/04/13 17:12:51  jeff
 * Applied "Darren O. Benham" <gecko@benham.net> spell check changes.
 * Fixed crash on Win32 with the new code.
 *
 * Revision 1.5  1999/01/07 01:07:48  paul
 * Fixed spell leaks.
 *
 * Revision 1.5  1999/01/07 01:07:48  paul
 * Fixed spell leaks.
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
 * Revision 1.42  1995/01/08  23:23:42  geoff
 * Support MSDOS_BINARY_OPEN when opening the hash file to read it in.
 *
 * Revision 1.41  1994/01/25  07:11:51  geoff
 * Get rid of all old RCS log lines in preparation for the 3.1 release.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <glib/gstdio.h>

#include "ispell_checker.h"
#include "msgs.h"

#define G_ICONV_INVALID (GIConv)-1

static bool g_iconv_is_valid(GIConv i)
{
  return (i != G_ICONV_INVALID);
}

#ifdef INDEXDUMP
static void	dumpindex P ((struct flagptr * indexp, int depth));
#endif /* INDEXDUMP */

int		gnMaskBits = 64;

/*!
 * \param hashname name of the hash file (dictionary)
 *
 * \return
 */
int ISpellChecker::linit (char *hashname)
{
	FILE*	fpHash;
		
    register int	i;
    register struct dent * dp;
    struct flagent *	entry;
    struct flagptr *	ind;
    int			nextchar, x;
    int			viazero;
    register ichar_t *	cp;

    if ((fpHash = g_fopen (hashname, "rb")) == NULL)
	{
		return (-1);
	}

    m_hashsize = fread (reinterpret_cast<char *>(&m_hashheader), 1, sizeof m_hashheader, fpHash);
    if (m_hashsize < static_cast<int>(sizeof(m_hashheader)))
	{
		if (m_hashsize < 0)
			fprintf (stderr, LOOKUP_C_CANT_READ, hashname);
		else if (m_hashsize == 0)
			fprintf (stderr, LOOKUP_C_NULL_HASH, hashname);
		else
			fprintf (stderr,
			  LOOKUP_C_SHORT_HASH (m_hashname, m_hashsize,
				static_cast<int>(sizeof m_hashheader)));
		return (-1);
	}
    else if (m_hashheader.magic != MAGIC)
	{
		fprintf (stderr,
		  LOOKUP_C_BAD_MAGIC (hashname, static_cast<unsigned int>(MAGIC),
			static_cast<unsigned int>(m_hashheader.magic)));
		return (-1);
	}
    else if (m_hashheader.magic2 != MAGIC)
	{
		fprintf (stderr,
		  LOOKUP_C_BAD_MAGIC2 (hashname, static_cast<unsigned int>(MAGIC),
			static_cast<unsigned int>(m_hashheader.magic2)));
		return (-1);
	}
/*  else if (hashheader.compileoptions != COMPILEOPTIONS*/
    else if ( 1 != 1
      ||  m_hashheader.maxstringchars != MAXSTRINGCHARS
      ||  m_hashheader.maxstringcharlen != MAXSTRINGCHARLEN)
	{
		fprintf (stderr,
		  LOOKUP_C_BAD_OPTIONS (static_cast<unsigned int>(m_hashheader.compileoptions),
			m_hashheader.maxstringchars, m_hashheader.maxstringcharlen,
			static_cast<unsigned int>(COMPILEOPTIONS), MAXSTRINGCHARS, MAXSTRINGCHARLEN));
		return (-1);
	}

	{
		m_hashtbl =
		 (struct dent *)
			calloc (static_cast<unsigned>(m_hashheader.tblsize), sizeof (struct dent));
		m_hashsize = m_hashheader.tblsize;
		m_hashstrings = static_cast<char *>(malloc(static_cast<unsigned>(m_hashheader.stringsize)));
	}
    m_numsflags = m_hashheader.stblsize;
    m_numpflags = m_hashheader.ptblsize;
    m_sflaglist = (struct flagent *)
      malloc ((m_numsflags + m_numpflags) * sizeof (struct flagent));
    if (m_hashtbl == NULL  ||  m_hashstrings == NULL  ||  m_sflaglist == NULL)
	{
		fprintf (stderr, LOOKUP_C_NO_HASH_SPACE);
		return (-1);
	}
    m_pflaglist = m_sflaglist + m_numsflags;

	{
		if( fread ( m_hashstrings, 1, static_cast<unsigned>(m_hashheader.stringsize), fpHash) 
			!= static_cast<size_t>(m_hashheader.stringsize) )
	    {
		    fprintf (stderr, LOOKUP_C_BAD_FORMAT);
			fprintf (stderr, "stringsize err\n" );
	    	return (-1);
	    }
		if ( m_hashheader.compileoptions & 0x04 )
		{
			if(  fread (reinterpret_cast<char *>(m_hashtbl), 1, static_cast<unsigned>(m_hashheader.tblsize) * sizeof(struct dent), fpHash)
		    	!= (static_cast<size_t>(m_hashheader.tblsize * sizeof (struct dent))))
		    {
			    fprintf (stderr, LOOKUP_C_BAD_FORMAT);
		    	return (-1);
		    }
		}
		else
		{
			for( x=0; x<m_hashheader.tblsize; x++ )
			{
				if(  fread ( reinterpret_cast<char*>(m_hashtbl+x), sizeof( struct dent)-sizeof( MASKTYPE ), 1, fpHash)
			    	!= 1)
			    {
				    fprintf (stderr, LOOKUP_C_BAD_FORMAT);
			    	return (-1);
			    }
			}	/*for*/
		}	/*else*/
	}
    if (fread (reinterpret_cast<char *>(m_sflaglist), 1,
	static_cast<unsigned>(m_numsflags+ m_numpflags) * sizeof (struct flagent), fpHash)
      != (m_numsflags + m_numpflags) * sizeof (struct flagent))
	{
		fprintf (stderr, LOOKUP_C_BAD_FORMAT);
		return (-1);
	}
    fclose (fpHash);

	{
		for (i = m_hashsize, dp = m_hashtbl;  --i >= 0;  dp++)
		{
			if (dp->word == (char *) -1)
				dp->word = NULL;
			else
				dp->word = &m_hashstrings [ reinterpret_cast<size_t>(dp->word) ];
			if (dp->next == (struct dent *) -1)
				dp->next = NULL;
			else
				dp->next = &m_hashtbl [ reinterpret_cast<size_t>(dp->next) ];
	    }
	}

    for (i = m_numsflags + m_numpflags, entry = m_sflaglist; --i >= 0; entry++)
	{
		if (entry->stripl)
			entry->strip = reinterpret_cast<ichar_t *>(&m_hashstrings[reinterpret_cast<size_t>(entry->strip)]);
		else
			entry->strip = NULL;
		if (entry->affl)
			entry->affix = reinterpret_cast<ichar_t *>(&m_hashstrings[reinterpret_cast<size_t>(entry->affix)]);
		else
			entry->affix = NULL;
	}
    /*
    ** Warning - 'entry' and 'i' are reset in the body of the loop
    ** below.  Don't try to optimize it by (e.g.) moving the decrement
    ** of i into the loop condition.
    */
    for (i = m_numsflags, entry = m_sflaglist;  i > 0;  i--, entry++)
	{
		if (entry->affl == 0)
		{
			cp = NULL;
			ind = &m_sflagindex[0];
			viazero = 1;
		}
		else
		{
			cp = entry->affix + entry->affl - 1;
			ind = &m_sflagindex[*cp];
			viazero = 0;
			while (ind->numents == 0  &&  ind->pu.fp != NULL)
			{
				if (cp == entry->affix)
				{
					ind = &ind->pu.fp[0];
					viazero = 1;
				}
				else
				{
					ind = &ind->pu.fp[*--cp];
					viazero = 0;
				}
			}
		}
		if (ind->numents == 0)
			ind->pu.ent = entry;
		ind->numents++;
		/*
		** If this index entry has more than MAXSEARCH flags in
		** it, we will split it into subentries to reduce the
		** searching.  However, the split doesn't make sense in
		** two cases:  (a) if we are already at the end of the
		** current affix, or (b) if all the entries in the list
		** have identical affixes.  Since the list is sorted, (b)
		** is true if the first and last affixes in the list
		** are identical.
		*/
		if (!viazero  &&  ind->numents >= MAXSEARCH
		  &&  icharcmp (entry->affix, ind->pu.ent->affix) != 0)
		{
			/* Sneaky trick:  back up and reprocess */
			entry = ind->pu.ent - 1; /* -1 is for entry++ in loop */
			i = m_numsflags - (entry - m_sflaglist);
			ind->pu.fp =
			  (struct flagptr *)
			calloc (static_cast<unsigned>(SET_SIZE + m_hashheader.nstrchars),
			  sizeof (struct flagptr));
			if (ind->pu.fp == NULL)
			{
				fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
				return (-1);
			}
			ind->numents = 0;
		}
	}
    /*
    ** Warning - 'entry' and 'i' are reset in the body of the loop
    ** below.  Don't try to optimize it by (e.g.) moving the decrement
    ** of i into the loop condition.
    */
    for (i = m_numpflags, entry = m_pflaglist;  i > 0;  i--, entry++)
	{
		if (entry->affl == 0)
	    {
			cp = NULL;
			ind = &m_pflagindex[0];
			viazero = 1;
	    }
		else
		{
			cp = entry->affix;
			ind = &m_pflagindex[*cp++];
			viazero = 0;
			while (ind->numents == 0  &&  ind->pu.fp != NULL)
			{
				if (*cp == 0)
				{
					ind = &ind->pu.fp[0];
					viazero = 1;
				}
				else
				{
					ind = &ind->pu.fp[*cp++];
					viazero = 0;
				}
			}
		}
		if (ind->numents == 0)
			ind->pu.ent = entry;
		ind->numents++;
		/*
		** If this index entry has more than MAXSEARCH flags in
		** it, we will split it into subentries to reduce the
		** searching.  However, the split doesn't make sense in
		** two cases:  (a) if we are already at the end of the
		** current affix, or (b) if all the entries in the list
		** have identical affixes.  Since the list is sorted, (b)
		** is true if the first and last affixes in the list
		** are identical.
		*/
		if (!viazero  &&  ind->numents >= MAXSEARCH
		  &&  icharcmp (entry->affix, ind->pu.ent->affix) != 0)
		{
			/* Sneaky trick:  back up and reprocess */
			entry = ind->pu.ent - 1; /* -1 is for entry++ in loop */
			i = m_numpflags - (entry - m_pflaglist);
			ind->pu.fp =
			  static_cast<struct flagptr *>(calloc(SET_SIZE + m_hashheader.nstrchars,
				sizeof (struct flagptr)));
			if (ind->pu.fp == NULL)
			{
				fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
				return (-1);
			}
			ind->numents = 0;
		}
	}
#ifdef INDEXDUMP
    fprintf (stderr, "Prefix index table:\n");
    dumpindex (m_pflagindex, 0);
    fprintf (stderr, "Suffix index table:\n");
    dumpindex (m_sflagindex, 0);
#endif
    if (m_hashheader.nstrchartype == 0)
		m_chartypes = NULL;
    else
	{
		m_chartypes = (struct strchartype *)
		  malloc (m_hashheader.nstrchartype * sizeof (struct strchartype));
		if (m_chartypes == NULL)
		{
			fprintf (stderr, LOOKUP_C_NO_LANG_SPACE);
			return (-1);
		}
		for (i = 0, nextchar = m_hashheader.strtypestart;
		  i < m_hashheader.nstrchartype;
		  i++)
		{
			m_chartypes[i].name = &m_hashstrings[nextchar];
			nextchar += strlen (m_chartypes[i].name) + 1;
			m_chartypes[i].deformatter = &m_hashstrings[nextchar];
			nextchar += strlen (m_chartypes[i].deformatter) + 1;
			m_chartypes[i].suffixes = &m_hashstrings[nextchar];
			while (m_hashstrings[nextchar] != '\0')
				nextchar += strlen (&m_hashstrings[nextchar]) + 1;
			nextchar++;
		}
	}

    initckch(NULL);   
   
    return (0);
}

#ifndef FREEP
#define FREEP(p)	do { if (p) free(p); } while (0)
#endif

/*!
 * \param wchars Characters in -w option, if any
 */
void ISpellChecker::initckch (char *wchars)
{
	register ichar_t    c;
	char                num[4];

	for (c = 0; c < static_cast<ichar_t>(SET_SIZE+ m_hashheader.nstrchars); ++c)
    {
		if (iswordch (c))
		{
			if (!mylower (c))
			{
				m_Try[m_Trynum] = c;
				++m_Trynum;
			}
		}
		else if (isboundarych (c))
		{
			m_Try[m_Trynum] = c;
			++m_Trynum;
		}
	}
	if (wchars != NULL)
    {
		while (m_Trynum < SET_SIZE  &&  *wchars != '\0')
		{
			if (*wchars != 'n'  &&  *wchars != '\\')
			{
				c = *wchars;
				++wchars;
			}
			else
			{
			    ++wchars;
			    num[0] = '\0';
			    num[1] = '\0';
			    num[2] = '\0';
			    num[3] = '\0';
			    if (isdigit (wchars[0]))
				{
				    num[0] = wchars[0];
				    if (isdigit (wchars[1]))
				    {
						num[1] = wchars[1];
						if (isdigit (wchars[2]))
							num[2] = wchars[2];
					}
				}
				if (wchars[-1] == 'n')
				{
				    wchars += strlen (num);
				    c = atoi (num);
				}
				else
				{
				    wchars += strlen (num);
				    c = 0;
				    if (num[0])
						c = num[0] - '0';
				    if (num[1])
				    {
						c <<= 3;
						c += num[1] - '0';
					}
					if (num[2])
					{
						c <<= 3;
						c += num[2] - '0';
					}
				}
			}
/*	    	c &= NOPARITY;*/
			if (!m_hashheader.wordchars[c])
			{
				m_hashheader.wordchars[c] = 1;
				m_hashheader.sortorder[c] = m_hashheader.sortval++;
				m_Try[m_Trynum] = c;
				++m_Trynum;
			}
		}
    }
}

/*
 * \param indexp
 */
void ISpellChecker::clearindex (struct flagptr *indexp)
{
    register int		i;
    for (i = 0;  i < SET_SIZE + m_hashheader.nstrchars;  i++, indexp++)
	{
		if (indexp->numents == 0 && indexp->pu.fp != NULL)
		{
		    clearindex(indexp->pu.fp);
			free(indexp->pu.fp);
		}
	}
}
	
#ifdef INDEXDUMP
static void dumpindex (indexp, depth)
    register struct flagptr *	indexp;
    register int		depth;
{
    register int		i;
    int				j;
    int				k;
    char			stripbuf[INPUTWORDLEN + 4 * MAXAFFIXLEN + 4];

    for (i = 0;  i < SET_SIZE + hashheader.nstrchars;  i++, indexp++)
	{
		if (indexp->numents == 0  &&  indexp->pu.fp != NULL)
	    {
			for (j = depth;  --j >= 0;  )
				putc (' ', stderr);
			if (i >= ' '  &&  i <= '~')
				putc (i, stderr);
			else
				fprintf (stderr, "0x%x", i);
			putc ('\n', stderr);
			dumpindex (indexp->pu.fp, depth + 1);
	    }
		else if (indexp->numents)
		{
			for (j = depth;  --j >= 0;  )
				putc (' ', stderr);
			if (i >= ' '  &&  i <= '~')
				putc (i, stderr);
			else
				fprintf (stderr, "0x%x", i);
			fprintf (stderr, " -> %d entries\n", indexp->numents);
			for (k = 0;  k < indexp->numents;  k++)
			{
				for (j = depth;  --j >= 0;  )
					putc (' ', stderr);
				if (indexp->pu.ent[k].stripl)
				{
					ichartostr (stripbuf, indexp->pu.ent[k].strip,
					  sizeof stripbuf, 1);
					fprintf (stderr, "     entry %d (-%s,%s)\n",
					  &indexp->pu.ent[k] - sflaglist,
					  stripbuf,
					  indexp->pu.ent[k].affl
						? ichartosstr (indexp->pu.ent[k].affix, 1) : "-");
				}
				else
					fprintf (stderr, "     entry %d (%s)\n",
					  &indexp->pu.ent[k] - sflaglist,
					  ichartosstr (indexp->pu.ent[k].affix, 1));
			}
		}
	}
}
#endif

/* n is length of s */

/*
 * \param s
 * \param dotree
 *
 * \return
 */
struct dent * ISpellChecker::ispell_lookup (ichar_t *s, int dotree)
{
    register struct dent *	dp;
    register char *		s1;
    char			schar[INPUTWORDLEN + MAXAFFIXLEN];

    dp = &m_hashtbl[hash (s, m_hashsize)];
    if (ichartostr (schar, s, sizeof schar, 1))
		fprintf (stderr, WORD_TOO_LONG (schar));
    for (  ;  dp != NULL;  dp = dp->next)
	{
		/* quick strcmp, but only for equality */
		s1 = dp->word;
		if (s1  &&  s1[0] == schar[0]  &&  strcmp (s1 + 1, schar + 1) == 0)
			return dp;
#ifndef NO_CAPITALIZATION_SUPPORT
		while (dp->flagfield & MOREVARIANTS)	/* Skip variations */
			dp = dp->next;
#endif
	}
	return NULL;
}

void ISpellChecker::alloc_ispell_struct()
{
	m_translate_in = 
	m_translate_out = G_ICONV_INVALID;
}

void ISpellChecker::free_ispell_struct()
{
	if (g_iconv_is_valid(m_translate_in))
		g_iconv_close (m_translate_in);
	if (g_iconv_is_valid(m_translate_out))
		g_iconv_close (m_translate_out);
}
