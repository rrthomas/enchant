/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2004 Remi Payette
 * Copyright (C) 2004 Francis James Franklin
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

#import <Cocoa/Cocoa.h>

#include "enchant-provider.h"

ENCHANT_PLUGIN_DECLARE("AppleSpell")

class AppleSpellChecker
{
 public:
	AppleSpellChecker();

	~AppleSpellChecker();

	void		parseConfigFile (const char * configFile);

	bool		checkWord (const char * word, size_t len, NSString * lang);
	char **		suggestWord (const char * const word, size_t len, size_t * nsug, NSString * lang);

	NSString *	requestDictionary (const char * const code);
 private:
	NSString *	dictionaryForCode (NSString * code);

	NSSpellChecker *	m_checker;
	NSMutableDictionary *	m_languages;
};

typedef struct
{
	AppleSpellChecker *	AppleSpell;
	NSString *		DictionaryName;
} AppleSpellDictionary;
