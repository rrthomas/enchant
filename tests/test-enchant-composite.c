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
 * Boston, MA 02110-1301, USA.
 *
 * In addition, as a special exception, Dom Lachowicz
 * gives permission to link the code of this program with
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers. If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so. If you do not wish to
 * do so, delete this exception statement from your version.
 */
#include<stdio.h>
#include<glib.h>
#include <stdlib.h>
#include <string.h>

#include "enchant.h"
#include "composite_provider.h"


void test1()
{
	//test string
	char tc[]="abcd";
	const char * err;

	//create a broker to use
	EnchantBroker *broker = enchant_broker_init ();

	//create a normal dictionary to benchmark results
	EnchantDict *eng_dict = enchant_broker_request_dict(broker,"en_US");

	if(!eng_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}
	
	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker,"en_US:fr_FR:"); 
	if(!cdict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}

	g_assert(enchant_dict_check(eng_dict,tc,strlen(tc)) == enchant_dict_check(cdict, tc, strlen(tc)));

	printf("test1 complete\n");
//cleanup
	g_free(broker);
	g_free(eng_dict);
	g_free(cdict);
}

void test2()
{
	//test string
	char tc[]="indien";
	const char * err;

	//create a broker to use
	EnchantBroker *broker = enchant_broker_init ();

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = enchant_broker_request_dict(broker,"en_US");

	if(!eng_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}
	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = enchant_broker_request_dict(broker,"fr_FR");

	if(!fr_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for fr_FR: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for fr_FR\n");
		return;
	}
	
	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker,"en_US:fr_FR:"); 
	if(!cdict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}

	g_assert(enchant_dict_check(eng_dict,tc,strlen(tc)) != enchant_dict_check(cdict, tc, strlen(tc)));

	g_assert(enchant_dict_check(fr_dict,tc,strlen(tc)) == enchant_dict_check(cdict, tc, strlen(tc)));

	printf("test2 complete\n");
//cleanup
	g_free(broker);
	g_free(eng_dict);
	g_free(fr_dict);
	g_free(cdict);
}

void test3()
{
	//test string
	char tc[]="‘abcdefghijklmnop";
	const char * err;
	char **sug_eng, **sug_fr, **sug_comp;
	int n_sug_eng,n_sug_fr,n_sug_comp;

	//create a broker to use
	EnchantBroker *broker = enchant_broker_init ();

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = enchant_broker_request_dict(broker,"en_US");

	if(!eng_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}
	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = enchant_broker_request_dict(broker,"fr_FR");

	if(!fr_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for fr_FR: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for fr_FR\n");
		return;
	}
	
	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker,"en_US:fr_FR:"); 
	if(!cdict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}

	sug_eng = enchant_dict_suggest(eng_dict,tc,strlen(tc),&n_sug_eng);
	sug_fr = enchant_dict_suggest(fr_dict,tc,strlen(tc),&n_sug_fr);
	sug_comp = enchant_dict_suggest(cdict,tc,strlen(tc),&n_sug_comp);

//just asserting the number of suggestions to begin with
	g_assert_cmpint(n_sug_eng, ==,0);
	g_assert_cmpint(n_sug_fr, ==,0);
	g_assert_cmpint(n_sug_comp, ==,0);

	printf("test3 complete\n");
//cleanup
	g_free(broker);
	g_free(eng_dict);
	g_free(fr_dict);
	g_free(cdict);
}

void test6()
{
	//test string
	char tc[]="‘teh";
	const char * err;
	char **sug_eng, **sug_fr, **sug_comp;
	int n_sug_eng,n_sug_fr,n_sug_comp;

	//create a broker to use
	EnchantBroker *broker = enchant_broker_init ();

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = enchant_broker_request_dict(broker,"en_US");

	if(!eng_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}
	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = enchant_broker_request_dict(broker,"fr_FR");

	if(!fr_dict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for fr_FR: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for fr_FR\n");
		return;
	}
	
	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker,"en_US:fr_FR:"); 
	if(!cdict)
	{
		err = enchant_broker_get_error (broker);
		if (err)
			fprintf (stderr, "Couldn't create dictionary for en_US: %s\n", err);
		else
			fprintf (stderr, "Couldn't create dictionary for en_US\n");
		return;
	}
	sug_eng = enchant_dict_suggest(eng_dict,tc,strlen(tc),&n_sug_eng);
	sug_fr = enchant_dict_suggest(fr_dict,tc,strlen(tc),&n_sug_fr);
	sug_comp = enchant_dict_suggest(cdict,tc,strlen(tc),&n_sug_comp);

//just asserting the number of suggestions to begin with
	g_assert_cmpint(n_sug_comp, ==, n_sug_eng+n_sug_fr);

	printf("test6 complete\n");
//cleanup
	g_free(broker);
	g_free(eng_dict);
	g_free(cdict);

}


int main(int argc, char **argv)
{

	   g_test_init(&argc,&argv,NULL);
           g_test_add_func("/test1",test1);
	   g_test_add_func("/test2",test2);
	   g_test_add_func("/test3",test3);
	   g_test_add_func("/test6",test6);
           g_test_run();
	   
return 0;
}

