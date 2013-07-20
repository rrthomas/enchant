/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003,2004 Dom Lachowicz
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <glib.h>


#include "enchant.h"
#include "enchant-provider.h"

#ifdef __cplusplus
extern "C" {
#endif

static int
composite_dict_check (EnchantDict * me, const char *const word, size_t len)
{
}

static char **
composite_dict_suggest (EnchantDict * me, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
}

static void
composite_dict_add_to_personal (EnchantDict * me,
			     const char *const word, size_t len)
{
}

static void
composite_dict_add_to_session (EnchantDict * me,
			    const char *const word, size_t len)
{
}

static void
composite_dict_store_replacement (EnchantDict * me,
			       const char *const mis, size_t mis_len,
			       const char *const cor, size_t cor_len)
{
}

/**
 * composite_provider_create_dict
 * @list_of_dicts: A non-null list of #EnchantDicts
 *
 * Remarks: creates a composite dictionary that composes of EnchantDicts in @list_of_dicts
 *          and maps all methods to composite provider methods for dictionary operations.
 */
EnchantDict *
composite_provider_create_dict (GSList* list_of_dicts)
{
	EnchantDict *dict;
	CompositeDict *comp_dict;

	g_return_if_fail (list_of_dicts);

	comp_dict = g_new0 (CompositeDict, 1);
	comp_dict->dict_list = list_of_dicts;

	dict = g_new0 (EnchantDict, 1);
	dict->user_data = (void *) comp_dict;

	dict->check = composite_dict_check;
	dict->suggest = composite_dict_suggest;
	dict->add_to_personal = composite_dict_add_to_personal;
	dict->add_to_session = composite_dict_add_to_session;
	dict->store_replacement = composite_dict_store_replacement;

	return dict;
}

#ifdef __cplusplus
}
#endif
