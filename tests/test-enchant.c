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

#include <stdio.h>
#include <stdlib.h>
#include "enchant.h"

static void
run_dict_tests (EnchantDict * dict)
{
  char **suggs;
  size_t n_suggs;
  size_t i, j;

  const char *check_checks[] = { "hello", "helllo" };
  const char *sugg_checks[] = { "helllo", "baad" };

  for (i = 0; i < (sizeof (check_checks) / sizeof (check_checks[0])); i++)
    {
      printf ("enchant_dict_check (%s): %d\n", check_checks[i],
	      enchant_dict_check (dict, check_checks[i],
				  strlen (check_checks[i])));
    }

  for (i = 0; i < (sizeof (sugg_checks) / sizeof (sugg_checks[0])); i++)
    {
      suggs =
	enchant_dict_suggest (dict, sugg_checks[i], strlen (sugg_checks[i]),
			      &n_suggs);

      printf ("enchant_dict_suggest(%s): %d\n", sugg_checks[i], n_suggs);
      for (j = 0; j < n_suggs; j++)
	{
	  printf ("\t=>%s\n", suggs[j]);
	}
    }
}

int
main (int argc, char **argv)
{
  EnchantBroker *broker;
  EnchantDict *dict;

  broker = enchant_broker_init ();

  dict = enchant_broker_request_dict (broker, "en_US");

  if (!dict) {
    fprintf (stderr, "Couldn't create dictionary for en_US\n");
  } else {
    run_dict_tests (dict);
  
    enchant_broker_release_dict (broker, dict);
  }

  enchant_broker_term (broker);

  return 0;
}
