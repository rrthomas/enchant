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
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.*
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef SPELL_H
#define SPELL_H

/*
  TODO stuff we need to do for this spell module:

  eliminate all the stderr fprintfs
  rip out the support for ICHAR_IS_CHAR
*/

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _sp_suggestions {
	int count;
	short *score;
	unsigned short **word;
} sp_suggestions;
   
int SpellCheckInit(char *hashname);
void SpellCheckCleanup(void);
int SpellCheckNWord16(const unsigned short *word16, int length);
int SpellCheckSuggestNWord16(const unsigned short *word16, int length, sp_suggestions *sg);

#ifdef __cplusplus
}
#endif
	
#endif /* SPELL_H */
