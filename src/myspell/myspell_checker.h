/* enchant
 * Copyright (C) 2003
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef MYSPELL_CHECKER_H
#define MYSPELL_CHECKER_H

#include "myspell.hxx"
#include <glib.h>

class MySpellChecker
{
public:
	MySpellChecker();
	~MySpellChecker();

	bool checkWord (const char *word, size_t len);
	char **suggestWord (const char* const word, size_t len, size_t *out_n_suggs);

	bool requestDictionary (const char * szLang);

private:
	GIConv  m_translate_in; /* Selected translation from/to Unicode */
	GIConv  m_translate_out;
	MySpell *myspell;
};

#endif /* MYSPELL_CHECKER_H */
