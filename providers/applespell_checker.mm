/* enchant
 * Copyright (C) 2004 Remi Payette
 * Copyright (C) 2004 Francis James Franklin
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib.h>
#include <gmodule.h>

#import <Cocoa/Cocoa.h>

#include "enchant-provider.h"

class AppleSpellChecker
{
 public:
	AppleSpellChecker();

	~AppleSpellChecker();

	void		parseConfigFile (const char * configFile);

	bool		checkWord (const char * word, size_t len, NSString * lang);
	char **		suggestWord (const char * const word, size_t len, size_t * nsug, NSString * lang);

	NSString *	requestDictionary (const char * const code);
	char **		listDictionaries (size_t *ndicts);
 private:
	NSString *	dictionaryForCode (NSString * code);
	NSString *	codeForDictionary (NSString * dict);
	char **		NSArrayToCArray (NSArray<NSString *> *array, size_t *nresult);

	NSSpellChecker *	m_checker;
	NSMutableDictionary *	m_languages;
	NSMutableDictionary *	m_dictionaries;
};

typedef struct
{
	AppleSpellChecker *	AppleSpell;
	NSString *		DictionaryName;
} AppleSpellDictionary;

AppleSpellChecker::AppleSpellChecker()
{
	m_checker = [NSSpellChecker sharedSpellChecker];

	m_languages = [[NSMutableDictionary alloc] initWithCapacity:16];
	m_dictionaries = [[NSMutableDictionary alloc] initWithCapacity:16];
}

AppleSpellChecker::~AppleSpellChecker()
{
	if (m_languages)
		{
			[m_languages release];
			m_languages = 0;
		}
	if (m_dictionaries)
		{
			[m_dictionaries release];
			m_dictionaries = 0;
		}
}

void AppleSpellChecker::parseConfigFile (const char * configFile)
{
	if (!m_languages || !m_dictionaries || !configFile)
		return;

	if (FILE * in = fopen (configFile, "r"))
		{
			char line[1024];
			char code[1024];
			char name[1024];
			char lang[1024];

			while (fgets (line, sizeof(line), in))
				if (sscanf (line, "%s %s %s", code, name, lang) == 3)
					{
						NSString * key      = [[NSString alloc] initWithUTF8String:code];
						NSString * value    = [[NSString alloc] initWithUTF8String:name];
						NSString * language = [[NSString alloc] initWithUTF8String:lang];

						if (key && value)
							{
								[m_languages setObject:value forKey:key];
								[m_dictionaries setObject:key forKey:value];
							}

						if (key)
							[key release];
						if (value)
							[value release];
						if (language)
							[language release];
					}
			fclose (in);
		}
}

char **AppleSpellChecker::NSArrayToCArray (NSArray<NSString *> *array, size_t *nresult)
{
	char ** result = 0;
	*nresult = 0;

	if (unsigned int count = [array count])
		{
			result = g_new0 (char *, static_cast<size_t>(count) + 1);
			if (result)
				{
					for (unsigned int i = 0; i < count; i++)
						{
							NSString *str = [array objectAtIndex:i];

							result[*nresult] = g_strdup ([str UTF8String]);

							if (result[*nresult])
								*nresult = *nresult + 1;
						}
				}
		}
	return result;
}

NSString * AppleSpellChecker::dictionaryForCode (NSString * code)
{
	if (!m_languages)
		return 0;

	return [m_languages objectForKey:code];
}

NSString * AppleSpellChecker::codeForDictionary (NSString * dict)
{
	if (!m_dictionaries)
		return 0;

	return [m_dictionaries objectForKey:dict];
}

bool AppleSpellChecker::checkWord (const char * word, size_t len, NSString * lang)
{
	if (!m_checker || !lang)
		return false;

	NSString * str = [[NSString alloc] initWithBytes:word length:len encoding:NSUTF8StringEncoding];
	if (!str)
		return false;

	[m_checker setLanguage:lang];

	NSRange result = [m_checker checkSpellingOfString:str startingAt:0];

	[str release];

	return (result.length ? true : false);
}

char ** AppleSpellChecker::suggestWord (const char * const word, size_t len, size_t * nsug, NSString * lang)
{
	if (!m_checker || !word || !len || !nsug || !lang)
		return 0;

	*nsug = 0;

	[m_checker setLanguage:lang];

	NSString * str = [[NSString alloc] initWithBytes:word length:len encoding:NSUTF8StringEncoding];
	if (!str)
		return 0;

	NSRange range = NSMakeRange(0, [str length]);
	NSArray<NSString *>* result = [m_checker guessesForWordRange:range inString:str language:lang inSpellDocumentWithTag:0];

	[str release];

	return NSArrayToCArray(result, nsug);
}

NSString * AppleSpellChecker::requestDictionary (const char * const code)
{
	if (!m_checker || !code)
		return 0;

	NSString * dictionary = dictionaryForCode ([NSString stringWithUTF8String:code]);
	if (dictionary)
		{
			NSString * language = [m_checker language];
			if (![m_checker setLanguage:dictionary])
				{
					dictionary = 0;
				}
			if (language)
				[m_checker setLanguage:language];
		}
	return dictionary;
}

char **AppleSpellChecker::listDictionaries (size_t *ndict)
{
	if (!m_checker)
		return 0;

	NSArray<NSString *> *availLanguages = [m_checker availableLanguages];
	NSMutableArray<NSString *> *availDicts = [NSMutableArray arrayWithCapacity:[availLanguages count]];

	for (NSString *string in availLanguages) {
		NSString *code = codeForDictionary(string);
		if (code)
			[availDicts addObject:code];
	}

	return NSArrayToCArray(availDicts, ndict);
}

/*
 * Enchant
 */

static char ** appleSpell_dict_suggest (EnchantDict * me, const char * const word, size_t len, size_t * out_n_suggs)
{
	@autoreleasepool {
		if (!me || !word || !len || !out_n_suggs)
			{
				return 0;
			}

		char ** result = 0;

		if (AppleSpellDictionary * ASD = static_cast<AppleSpellDictionary *>(me->user_data))
			{
				result = ASD->AppleSpell->suggestWord (word, len, out_n_suggs, ASD->DictionaryName);
			}

		return result;
	}
}

static int appleSpell_dict_check (EnchantDict * me, const char * const word, size_t len)
{
	@autoreleasepool {
		if (!me || !word || !len)
			{
				return 0;
			}

		int result = 0;

		if (AppleSpellDictionary * ASD = static_cast<AppleSpellDictionary *>(me->user_data))
			{
				result = ASD->AppleSpell->checkWord (word, len, ASD->DictionaryName);
			}
		return result;
	}
}

static EnchantDict * appleSpell_provider_request_dict (EnchantProvider * me, const char * const tag)
{
	@autoreleasepool {
		AppleSpellChecker * checker = static_cast<AppleSpellChecker *>(me->user_data);

		if (!me || !tag || !checker)
			{
				return 0;
			}

		AppleSpellDictionary * ASD = g_new0 (AppleSpellDictionary, 1);
		if (!ASD)
			{
				return 0;
			}

		ASD->AppleSpell     = checker;
		ASD->DictionaryName = checker->requestDictionary (tag);

		if (!ASD->DictionaryName)
			{
				g_free (ASD);
				return 0;
			}

		[ASD->DictionaryName retain];

		EnchantDict * dict = enchant_broker_new_dict (me->owner);

		if (!dict)
			{
				return 0;
			}

		dict->check     = appleSpell_dict_check;
		dict->suggest   = appleSpell_dict_suggest;
		dict->user_data = (void *) ASD;

		return dict;
	}
}

static void appleSpell_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
	@autoreleasepool {
		if (dict)
			{
				AppleSpellDictionary * ASD = static_cast<AppleSpellDictionary *>(dict->user_data);
				if (ASD)
					{
						[ASD->DictionaryName release];
						g_free (ASD);
					}
			}
	}
}

static int appleSpell_provider_dictionary_exists (EnchantProvider * me, const char * const tag)
{
	@autoreleasepool {
		int result = 0;

		AppleSpellChecker * checker = static_cast<AppleSpellChecker *>(me->user_data);
		if (checker)
			result = (checker->requestDictionary (tag) ? 1 : 0);

		return result;
	}
}

static char **appleSpell_provider_list_dicts (EnchantProvider *me, size_t *n_dicts)
{
	@autoreleasepool {
		char **result = 0;

		AppleSpellChecker * checker = static_cast<AppleSpellChecker *>(me->user_data);
		if (checker)
			result = checker->listDictionaries (n_dicts);
		else
			*n_dicts = 0;

		return result;
	}
}

static void appleSpell_provider_dispose (EnchantProvider * me)
{
	@autoreleasepool {
		if (me)
			{
				AppleSpellChecker * checker = static_cast<AppleSpellChecker *>(me->user_data);
				if (checker)
					delete checker;
			}
	}
}

static const char * appleSpell_provider_identify (EnchantProvider * me)
{
	return "AppleSpell";
}

static const char * appleSpell_provider_describe (EnchantProvider * me)
{
	return "AppleSpell Provider";
}

extern "C" {
	EnchantProvider *init_enchant_provider (void)
	{
		@autoreleasepool {
			EnchantProvider * provider = enchant_provider_new ();
			if (!provider)
				{
					return 0;
				}

			provider->dispose           = appleSpell_provider_dispose;
			provider->request_dict      = appleSpell_provider_request_dict;
			provider->dispose_dict      = appleSpell_provider_dispose_dict;
			provider->dictionary_exists = appleSpell_provider_dictionary_exists;
			provider->identify          = appleSpell_provider_identify;
			provider->describe          = appleSpell_provider_describe;
			provider->list_dicts        = appleSpell_provider_list_dicts;

			AppleSpellChecker * checker = 0;
			try
				{
					checker = new AppleSpellChecker;
				}
			catch (...)
				{
					checker = 0;
				}
			if (checker)
				{
					provider->user_data = (void *) checker;
				}
			else
				{
					g_free (provider);
					provider = 0;
				}

			return provider;
		}
	}

	static bool s_bReloadSelf = true;

	void configure_enchant_provider (EnchantProvider * me, const char * module_dir)
	{
		@autoreleasepool {
			if (!me || !module_dir)
				{
					return;
				}

			AppleSpellChecker * checker = static_cast<AppleSpellChecker *>(me->user_data);
			if (checker)
				for (GSList *iter = enchant_get_conf_dirs (); iter; iter = iter->next)
					{
						if (gchar * configFile = g_build_filename ((gchar *)iter->data, "AppleSpell.config", NULL))
						{
							checker->parseConfigFile (configFile);
							g_free (configFile);
						}
					}

		}

		return;
	}
}
