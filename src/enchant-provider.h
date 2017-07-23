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
 * Boston, MA 02110-1301, USA.
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

#ifndef ENCHANT_PROVIDER_H
#define ENCHANT_PROVIDER_H

#include <enchant.h>
#include <glib.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct str_enchant_provider EnchantProvider;

/**
 * enchant_get_user_language
 *
 * Returns a char string giving the current language.
 * Defaults to "en" if no language or locale can be found, or
 * locale is C.
 *
 * The returned string should be free'd with free.
 */
char *enchant_get_user_language(void);

char *enchant_get_user_config_dir (void);
GSList *enchant_get_conf_dirs (void);

/**
 * enchant_get_prefix_dir
 *
 * Returns a string giving the location of the base directory
 * of the enchant installation.  This corresponds roughly to
 * the --prefix option given to ./configure when enchant is
 * compiled, except it is determined at runtime based on the location
 * of the enchant library.
 *
 * Returns: the prefix dir. Must be free'd.
 *
 */
char *enchant_get_prefix_dir(void);

/**
 * enchant_dict_set_error
 * @dict: A non-null dictionary
 * @err: A non-null error message
 *
 * Sets the current runtime error to @err. This API is private to the
 * providers.
 */
void enchant_dict_set_error (EnchantDict * dict, const char * const err);

/**
 * enchant_provider_set_error
 * @provider: A non-null provider
 * @err: A non-null error message
 *
 * Sets the current runtime error to @err. This API is private to
 * the providers.
 */
void enchant_provider_set_error (EnchantProvider * provider, const char * const err);

struct str_enchant_dict
{
	void *user_data;
	void *enchant_private_data;

	int (*check) (struct str_enchant_dict * me, const char *const word,
			  size_t len);
	
	/* returns utf8*/
	char **(*suggest) (struct str_enchant_dict * me,
			   const char *const word, size_t len,
			   size_t * out_n_suggs);
	
	void (*add_to_personal) (struct str_enchant_dict * me,
				 const char *const word, size_t len);
	
	void (*add_to_session) (struct str_enchant_dict * me,
				const char *const word, size_t len);
	
	void (*store_replacement) (struct str_enchant_dict * me,
				   const char *const mis, size_t mis_len,
				   const char *const cor, size_t cor_len);
	
	void (*add_to_exclude) (struct str_enchant_dict * me,
				const char *const word, size_t len);

	const char * (*get_extra_word_characters) (struct str_enchant_dict * me);

	int (*is_word_character) (struct str_enchant_dict * me,
				  uint32_t uc_in, size_t n);
};
	
struct str_enchant_provider
{
	void *user_data;
	void *enchant_private_data;
	EnchantBroker * owner;
	
	void (*dispose) (struct str_enchant_provider * me);
	
	EnchantDict *(*request_dict) (struct str_enchant_provider * me,
				      const char *const tag);
	
	void (*dispose_dict) (struct str_enchant_provider * me,
				  EnchantDict * dict);
	
	int (*dictionary_exists) (struct str_enchant_provider * me,
				  const char *const tag);

	/* returns utf8*/
	const char * (*identify) (struct str_enchant_provider * me);
	/* returns utf8*/
	const  char * (*describe) (struct str_enchant_provider * me);

	char ** (*list_dicts) (struct str_enchant_provider * me,
			       size_t * out_n_dicts);
};

#ifdef __cplusplus
}
#endif

#endif /* ENCHANT_H */
