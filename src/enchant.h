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
 */

#ifndef ENCHANT_H
#define ENCHANT_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ENCHANT_MODULE_EXPORT(x) __declspec(dllexport) x
#else
#define ENCHANT_MODULE_EXPORT(x) x
#endif

typedef struct str_enchant_broker EnchantBroker;
typedef struct str_enchant_dict EnchantDict;
typedef struct str_enchant_provider EnchantProvider;

ENCHANT_MODULE_EXPORT (int)
     enchant_dict_check (EnchantDict * dict, const char *const word, size_t len);
ENCHANT_MODULE_EXPORT (char **)
     enchant_dict_suggest (EnchantDict * dict, const char *const word,
			   size_t len, size_t * out_n_suggs);
ENCHANT_MODULE_EXPORT (void)
     enchant_dict_add_to_personal (EnchantDict * dict, const char *const word,
				   size_t len);
ENCHANT_MODULE_EXPORT (void)
     enchant_dict_add_to_session (EnchantDict * dict, const char *const word,
				  size_t len);
ENCHANT_MODULE_EXPORT (void)
     enchant_dict_store_replacement (EnchantDict * dict,
				     const char *const mis, size_t mis_len,
				     const char *const cor, size_t cor_len);
ENCHANT_MODULE_EXPORT (void)
     enchant_dict_free_suggestions (EnchantDict * dict, char **suggestions);

ENCHANT_MODULE_EXPORT (EnchantBroker *) 
     enchant_broker_init (void);

ENCHANT_MODULE_EXPORT (void)
     enchant_broker_term (EnchantBroker * broker);

ENCHANT_MODULE_EXPORT (EnchantDict *)
     enchant_broker_request_dict (EnchantBroker * broker, const char *const tag);
ENCHANT_MODULE_EXPORT (void)
     enchant_broker_release_dict (EnchantBroker * broker, EnchantDict * dict);

struct str_enchant_dict
{
	void *user_data;
	EnchantProvider *owner;
	
	int (*check) (struct str_enchant_dict * me, const char *const word,
		      size_t len);
	
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
	
	void (*free_suggestions) (struct str_enchant_dict * me,
				  char **str_list);
	
	void (*_reserved_func1) (void);
	void (*_reserved_func2) (void);
	void (*_reserved_func3) (void);
	void (*_reserved_func4) (void);
	void (*_reserved_func5) (void);
};
	
struct str_enchant_provider
{
	void *user_data;
	void *enchant_private_data;
	
	void (*dispose) (struct str_enchant_provider * me);
	
	EnchantDict *(*request_dict) (struct str_enchant_provider * me,
				      const char *const tag);
	
	void (*dispose_dict) (struct str_enchant_provider * me,
			      EnchantDict * dict);
	
	void (*_reserved_func1) (void);
	void (*_reserved_func2) (void);
	void (*_reserved_func3) (void);
	void (*_reserved_func4) (void);
	void (*_reserved_func5) (void);
};

#ifdef __cplusplus
}
#endif

#endif /* ENCHANT_H */
