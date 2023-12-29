/* Copyright (C) 2013 Vidhoon Vishwanathan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "enchant.h"
#include "enchant-provider.h"


static EnchantDict *request_dict(EnchantBroker *broker, const char *tag) {
	EnchantDict *dict;
	if (strchr(tag, ':') == NULL)
		dict = enchant_broker_request_dict(broker, tag);
	else
		dict = enchant_broker_request_composite_dict(broker, tag);
	if (dict == NULL) {
		const char *err = enchant_broker_get_error(broker);
		fprintf(stderr, "Couldn't create dictionary for %s", tag);
		if (err)
			fprintf(stderr, ": %s\n", err);
		fprintf(stderr, "\n");
	}
	return dict;
}


static void test1() {
	EnchantBroker *broker = enchant_broker_init();

	//test string
	char tc[]="abcd";

	//create a normal dictionary to benchmark results
	EnchantDict *eng_dict = request_dict(broker, "en_US");
	g_assert_true(eng_dict != NULL);

	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = request_dict(broker, "en_US:fr_FR");
	g_assert_true(cdict != NULL);

	g_assert_cmpint(enchant_dict_check(eng_dict, tc, strlen(tc)), ==, enchant_dict_check(cdict, tc, strlen(tc)));

	printf("test1 complete\n");
	enchant_broker_free(broker);
}

static void test2() {
	EnchantBroker *broker = enchant_broker_init();

	//test string
	char tc[]="indien";

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = request_dict(broker, "en_US");
	g_assert_true(eng_dict != NULL);
	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = request_dict(broker, "fr_FR");
	g_assert_true(fr_dict != NULL);

	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = request_dict(broker, "en_US:fr_FR");
	g_assert_true(cdict != NULL);

	g_assert_cmpint(enchant_dict_check(eng_dict, tc, strlen(tc)), !=, enchant_dict_check(cdict, tc, strlen(tc)));
	g_assert_cmpint(enchant_dict_check(fr_dict, tc, strlen(tc)), ==, enchant_dict_check(cdict, tc, strlen(tc)));

	printf("test2 complete\n");
	enchant_broker_free(broker);
}

static void test3() {
	EnchantBroker *broker = enchant_broker_init();

	//test string
	char tc[]="‘abcdefghijklmnop";

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = enchant_broker_request_dict(broker, "en_US");
	g_assert_true(eng_dict != NULL);
	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = enchant_broker_request_dict(broker, "fr_FR");
	g_assert_true(fr_dict != NULL);

	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker, "en_US:fr_FR");
	g_assert_true(cdict != NULL);
	
	size_t n_sug_eng, n_sug_fr, n_sug_comp;
	char **sug_eng = enchant_dict_suggest(eng_dict, tc, strlen(tc), &n_sug_eng);
	char **sug_fr = enchant_dict_suggest(fr_dict, tc, strlen(tc), &n_sug_fr);
	char **sug_comp = enchant_dict_suggest(cdict, tc, strlen(tc), &n_sug_comp);

	//just asserting the number of suggestions to begin with
	g_assert_cmpint(n_sug_eng, ==, 0);
	g_assert_cmpint(n_sug_fr, ==, 0);
	g_assert_cmpint(n_sug_comp, ==, 0);

	printf("test3 complete\n");
	enchant_broker_free(broker);
}

static void test4() {
	EnchantBroker *broker = enchant_broker_init();

	//test string
	char tc[]="‘teh";

	//create a normal dictionary for en_US
	EnchantDict *eng_dict = enchant_broker_request_dict(broker, "en_US");
	g_assert_true(eng_dict != NULL);

	//create a normal dictionary for fr_FR
	EnchantDict *fr_dict = enchant_broker_request_dict(broker, "fr_FR");
	g_assert_true(fr_dict != NULL);

	//create a composite dictionary Eng,Fr
	EnchantDict *cdict = enchant_broker_request_composite_dict(broker, "en_US:fr_FR");
	g_assert_true(cdict != NULL);

	size_t n_sug_eng,n_sug_fr,n_sug_comp;
	char **sug_eng = enchant_dict_suggest(eng_dict, tc, strlen(tc), &n_sug_eng);
	char **sug_fr = enchant_dict_suggest(fr_dict, tc, strlen(tc), &n_sug_fr);
	char **sug_comp = enchant_dict_suggest(cdict, tc, strlen(tc), &n_sug_comp);

	//just asserting the number of suggestions to begin with
	g_assert_cmpint(n_sug_comp, ==, n_sug_eng + n_sug_fr);

	printf("test4 complete\n");
	enchant_broker_free(broker);
}


int main(int argc, char **argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/test1", test1);
	g_test_add_func("/test2", test2);
	g_test_add_func("/test3", test3);
	g_test_add_func("/test4", test4);
	g_test_run();

	return 0;
}
