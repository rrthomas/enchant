/* enchant
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2016-2024 Reuben Thomas
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders
 * give permission to link the code of this program with
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#ifndef ENCHANT_H
#define ENCHANT_H

#include <stdint.h> /* for uint32_t */
#include <sys/types.h> /* for size_t, ssize_t */


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _EnchantBroker EnchantBroker;
typedef struct _EnchantDict   EnchantDict;

const char *enchant_get_version (void);

/**
 * Note: strings are all UTF-8, except for file names and paths, which are
 * in the GLib file name encoding.
 */

/**
 * enchant_broker_init
 *
 * Returns: A new broker object capable of requesting
 * dictionaries from providers.
 */
EnchantBroker *enchant_broker_init (void);

/**
 * enchant_broker_free
 * @broker: A non-null #EnchantBroker
 *
 * Destroys the broker object. Must only be called once per broker init
 */
void enchant_broker_free (EnchantBroker * broker);

/**
 * enchant_broker_request_dict
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag or tags you wish to request a dictionary
 *     for ("en_US", "de_DE", "en_US,fr_FR", ...)
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be found.
 * The default personal wordlist file is used.
 */
EnchantDict *enchant_broker_request_dict (EnchantBroker * broker, const char *const tag);

/**
 * enchant_broker_request_dict_with_pwl
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag or tags you wish to request a dictionary
 *     for ("en_US", "de_DE", "en_US,fr_FR", ...)
 * @pwl: The full path of a personal wordlist file, or %null to default to
 *     "TAG.dic" in the user's configuration directory, where TAG is the
 *     first tag in @tag.
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be
 * found, or if the PWL could not be opened.
 */
EnchantDict *enchant_broker_request_dict_with_pwl (EnchantBroker * broker, const char *const tag, const char *pwl);

/**
 * enchant_broker_request_pwl_dict
 * @pwl: The full path of a personal wordlist file
 *
 * Returns: An #EnchantDict, or %null if no suitable dictionary could be
 * found, or if the PWL could not be opened.
 */
EnchantDict *enchant_broker_request_pwl_dict (EnchantBroker * broker, const char *const pwl);

/**
 * enchant_broker_free_dict
 * @broker: A non-null #EnchantBroker
 * @dict: A non-null #EnchantDict
 *
 * Frees the dictionary when you are finished with it. Must be called at
 * most once per #EnchantDict.
 */
void enchant_broker_free_dict (EnchantBroker * broker, EnchantDict * dict);

/**
 * enchant_broker_dict_exists
 * @broker: A non-null #EnchantBroker
 * @tag: The non-null language tag you wish to request a dictionary for
 *     ("en_US", "de_DE", ...)
 *
 * Returns: 1 if the dictionary exists, and 0 otherwise.
 */
int enchant_broker_dict_exists (EnchantBroker * broker, const char * const tag);

/**
 * enchant_broker_set_ordering
 * @broker: A non-null #EnchantBroker
 * @tag: A non-null language tag (en_US)
 * @ordering: A non-null ordering (nuspell,aspell,hunspell,hspell)
 *
 * Declares a preference of dictionaries to use for the language
 * referred to by @tag; see enchant(5) for more details.
 */
void enchant_broker_set_ordering (EnchantBroker * broker,
				  const char * const tag,
				  const char * const ordering);
/**
 * enchant_broker_get_error
 * @broker: A non-null broker
 *
 * Returns a possibly-invalid UTF-8 string describing the last error, or
 * %null.
 * WARNING: error is transient and is likely cleared as soon as the
 * next broker operation happens.
 */
const char *enchant_broker_get_error (EnchantBroker * broker);

/**
 * EnchantBrokerDescribeFn
 * @provider_name: The provider's identifier, such as "hunspell" or
 *     "aspell".
 * @provider_desc: A description of the provider, such as "Aspell 0.53".
 * @provider_dll_file: The provider's DLL filename.
 * @user_data: Supplied user data, or %null if you don't care
 *
 * Callback used to enumerate and describe Enchant's various providers.
 */
typedef void (*EnchantBrokerDescribeFn) (const char * const provider_name,
					 const char * const provider_desc,
					 const char * const provider_dll_file,
					 void * user_data);

/**
 * enchant_broker_describe
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantBrokerDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the Enchant providers and tells you some rudimentary
 * information about them.
 */
void enchant_broker_describe (EnchantBroker * broker,
			      EnchantBrokerDescribeFn fn,
			      void * user_data);

/**
 * enchant_dict_check
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to check
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 *
 * Returns: 0 if the word is correctly spelled, positive if not, negative on
 * invalid arguments or other error.
 */
int enchant_dict_check (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_suggest
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to find suggestions for
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 * @out_n_suggs: The location in which to store the number of suggestions
 *     returned, or %null
 *
 * Returns: A %null terminated list encoded suggestions, or %null if there
 * are no suggestions, or if any of the pre-conditions is not met.
 */
char **enchant_dict_suggest (EnchantDict * dict, const char *const word,
			     ssize_t len, size_t * out_n_suggs);

/**
 * enchant_dict_add
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your personal dictionary
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 *
 * The word is also added to the session. The word is removed from the exclude
 * dictionary if present there.
 */
void enchant_dict_add (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_add_to_session
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to this session
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 */
void enchant_dict_add_to_session (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_remove
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to add to your exclude dictionary and
 *     remove from the personal dictionary
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 *
 * The word is also removed from the session.
 */
void enchant_dict_remove (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_remove_from_session
 * @dict: A non-null #EnchantDict
 * @word: The non-null word you wish to exclude from this session
 * @len: The length of @word in bytes, or -1 for strlen(@word)
 */
void enchant_dict_remove_from_session (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_is_added
 * @dict: A non-null #EnchantDict
 * @word: The word you wish to check
 * @len: the length of @word in bytes, or -1 for strlen(@word)
 *
 * Returns: 1 if the word is valid in the session, 0 if not.
 */
int enchant_dict_is_added (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_is_removed
 * @dict: A non-null #EnchantDict
 * @word: The word you wish to check
 * @len: the length of @word in bytes, or -1 for strlen(@word)
 *
 * Returns: 1 if the word is specifically excluded from the session, 0 if
 * not; that is, it is in the exclude dictionary or in the session exclude
 * list, and has not been added to the session include list.
 */
int enchant_dict_is_removed (EnchantDict * dict, const char *const word, ssize_t len);

/**
 * enchant_dict_store_replacement
 * @dict: A non-null #EnchantDict
 * @mis: The non-null word for which you wish to add a correction
 * @mis_len: The length of @mis in bytes, or -1 for strlen(@mis)
 * @cor: The non-null correction word
 * @cor_len: The length of @cor in bytes, or -1 for strlen(@cor)
 *
 * Deprecated: 2.5.0: does nothing; the API is retained only for backwards
 * compatibility.
 */
void enchant_dict_store_replacement (EnchantDict * dict,
				     const char *const mis, ssize_t mis_len,
				     const char *const cor, ssize_t cor_len);

/**
 * enchant_dict_free_string_list
 * @dict: A non-null #EnchantDict
 * @string_list: A non-null string list returned from enchant_dict_suggest
 *
 * Releases the string list.
 */
void enchant_dict_free_string_list (EnchantDict * dict, char **string_list);

/**
 * enchant_dict_get_error
 * @dict: A non-null #EnchantDict
 *
 * Returns: a possibly-invalid UTF-8 string describing the last exception,
 * or %null.
 * WARNING: error is transient. It will likely be cleared as soon as
 * the next dictionary operation is called.
 */
const char *enchant_dict_get_error (EnchantDict * dict);

/**
 * enchant_dict_get_extra_word_characters
 * @dict: A non-null #EnchantDict
 *
 * Returns: a string containing the non-letter characters allowed in a word,
 * e.g. "01234567890’-". If hyphen occurs, it will be last, so that the
 * string can be appended to a character class used to match word
 * characters.
 *
 * Words containing non-letters not in this string will automatically be
 * rejected by Enchant.
 *
 * Note that for some back-ends the result may be a guess, in which case it
 * may include characters not actually allowed in the given dictionary.
 */
const char *enchant_dict_get_extra_word_characters (EnchantDict * dict);

/**
 * enchant_dict_is_word_character
 * @dict: An #EnchantDict, or %null
 * @uc: A Unicode code-point
 * @n: An integer: 0 if the character is at the start of a word, 1 if it is
 *     in the middle, or 2 if at the end.
 *
 * Returns: 1 if the given character is valid at the given position,
 * otherwise 0.
 *
 * One way to match a complete word is to check that the first character
 * matches with n == 0, then proceed matching characters with n == 1 until
 * failure, then proceed backwards until a character matches with n == 2.
 *
 * Note that for some back-ends the result may be a guess, in which case it
 * may allow characters not actually allowed in the given dictionary.
 *
 * If @dict is %null, a built-in implementation is used (FIXME: We should
 * document behavior for this). If @n is not 0, 1 or 2, then 0 is returned.
 */
int enchant_dict_is_word_character (EnchantDict * dict, uint32_t uc, size_t n);

/**
 * EnchantDictDescribeFn
 * @lang_tag: The dictionary's language tag (e.g. en_US, de_AT, ...)
 * @provider_name: The provider's name (e.g. "Aspell")
 * @provider_desc: The provider's description (e.g. "Aspell 0.50.3")
 * @provider_file: The full path of the dynamic library from which this
 *     dictionary's provider was loaded
 * @user_data: User data pointer, or %null
 *
 * Callback used to describe a dictionary.
 */
typedef void (*EnchantDictDescribeFn) (const char * const lang_tag,
				       const char * const provider_name,
				       const char * const provider_desc,
				       const char * const provider_file,
				       void * user_data);

/**
 * enchant_dict_describe
 * @broker: A non-null #EnchantDict
 * @dict: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Describes a dictionary.
 */
void enchant_dict_describe (EnchantDict * dict,
			    EnchantDictDescribeFn fn,
			    void * user_data);

/**
 * enchant_broker_list_dicts
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantDictDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the dictionaries available from all Enchant providers.
 */
void enchant_broker_list_dicts (EnchantBroker * broker,
				EnchantDictDescribeFn fn,
				void * user_data);

/**
 * enchant_set_prefix_dir
 *
 * Sets the prefix dir. This overrides any auto-detected value, and can also
 * be used on systems or installations where auto-detection does not work.
 */
void enchant_set_prefix_dir(const char *);

#ifdef __cplusplus
}
#endif

#endif /* ENCHANT_H */
