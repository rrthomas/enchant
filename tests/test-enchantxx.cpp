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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enchant.h"
#include "enchant++.h"

using namespace enchant;

static void
enumerate_dicts (const char * name,
		 const char * desc,
		 const char * file,
		 void * ud)
{
	printf ("%s: '%s' (%s)\n", name, desc, file);
}

static void
run_dict_tests (Broker::Dict * dict)
{
	std::vector<std::string> suggs;
	size_t i, j;
	
	const char *check_checks[] = { "hello", "helllo" };
	const char *sugg_checks[] = { "helllo", "taag" };
	
	for (i = 0; i < (sizeof (check_checks) / sizeof (check_checks[0])); i++)
		{
			printf ("enchant_dict_check (%s): %d\n", check_checks[i],
				dict->check (check_checks[i]) == false);
		}
	
	for (i = 0; i < (sizeof (sugg_checks) / sizeof (sugg_checks[0])); i++)
		{
			dict->suggest (sugg_checks[i], suggs);
			
			printf ("enchant_dict_suggest(%s): %d\n", sugg_checks[i], suggs.size());
			for (j = 0; j < suggs.size(); j++)
				{
					printf ("\t=>%s\n", suggs[j].c_str());
				}
		}
}

int
main (int argc, char **argv)
{
	Broker *broker;
	Broker::Dict *dict;
	
	broker = Broker::instance ();
	
	dict = broker->request_dict ("en_US");
	
	if (!dict) 
		{
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		} 
	else 
		{
			run_dict_tests (dict);
			
			broker->describe (enumerate_dicts, NULL);
			delete dict;
		}
	
	return 0;
}
