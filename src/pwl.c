/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
 * Copyright (C) 2003, 2004 Dom Lachowicz
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
 * non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU Lesser General Public License in all
 * respects for all of the code used other than said providers.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/**
 *
 *  This file implements personal word list (PWL) dictionaries in the
 *  type EnchantPWL.
 *
 *  Under the hood, a PWL is stored as a Trie.  Checking strings for
 *  correctness and making suggestions is done by traversing the Trie
 *  while allowing a certain number of miss-steps.  Due to the prefix
 *  compression of the Trie, this allows all strings in the PWL within
 *  a given edit distance of the target word to be enumerated quite
 *  efficiently.
 *
 *  Ideas for the future:
 *
 *     - Order the suggestions first by edit distance, then by
 *       soundex key, to put the most similar sounding suggestions
 *       at the front of the list.  Would need a "soundex" that is
 *       general enough to handle languages other than English.
 *
 *     - iterative deepending to find suggestions, rather than a straight
 *       search to depth three.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "pwl.h"

#if defined(HAVE_FLOCK) || defined(HAVE_LOCKF)
#include <unistd.h>
#include <sys/file.h>
#endif /* HAVE_FLOCK || HAVE_LOCKF */

#define ENCHANT_PWL_MAX_ERRORS 3
#define ENCHANT_PWL_MAX_SUGGS 15

/*  A PWL dictionary is stored as a Trie-like data structure EnchantTrie.
 *  The EnchantTrie datatype is completely recursive - all child nodes
 *  are simply EnchantTrie pointers.  This means that all functions 
 *  that potentially modify a trie need to return the modified trie,
 *  as additional memory may have been allocated.
 *
 *  The empty trie is simply the null pointer.  If a trie contains
 *  a single string, it is recorded in the "value" attribute and
 *  "subtries" is set to NULL.  When two or more strings are contained,
 *  "value" is NULL and "subtries" is a GHashTable mapping the first
 *  character of each string to the subtrie containing the remainder of
 *  that string.
 *
 *  All strings stored in the Trie are assumed to be in UTF format.
 *  Branching is done on unicode characters, not individual bytes.
 */
typedef struct str_enchant_trie EnchantTrie;
struct str_enchant_trie
{
	char* value;           /* final string found under this node */
	GHashTable* subtries;  /* Maps characters to subtries */
};

struct str_enchant_pwl
{
	EnchantTrie* trie;
	char * filename;
	GHashTable *words_in_trie;
};

/* Special Trie node indicating the end of a string */
static EnchantTrie n_EOSTrie;
static EnchantTrie* EOSTrie = &n_EOSTrie;

/*  The EnchantTrieMatcher structure maintains the state necessary to
 *  search for matching strings within an EnchantTrie.  It includes a
 *  callback function which will be called with each matching string
 *  as it is found.  The arguments to this function are:
 *
 *      - a freshly-allocated copy of the matching string, which must
 *        be freed by external code
 *      - the EnchantTrieMatcher object, giving the context of the match
 *        (e.g. number of errors)
 */
typedef struct str_enchant_trie_matcher EnchantTrieMatcher;
struct str_enchant_trie_matcher
{
	int num_errors;		/* Num errors encountered so far. */
	int max_errors;		/* Max errors before search should terminate */

	const char* word;	/* Word being searched for */
	ssize_t word_pos;	/* Current position in the word */

	char* path;		/* Path taken through the trie so far */
	ssize_t path_len;	/* Length of allocated path string */
	ssize_t path_pos;	/* Current end pos of path string */

	void (*cbfunc)(char*,EnchantTrieMatcher*); /* callback func */
	void* cbdata;		/* Private data for use by callback func */
};


/*  To allow the list of suggestions to be built up an item at a time,
 *  its state is maintained in an EnchantSuggList object.
 */
typedef struct str_enchant_sugg_list
{
	char** suggs;
	int* sugg_errs;
	size_t n_suggs;
} EnchantSuggList;


/*
 *   Function Prototypes
 */

static void enchant_pwl_add_to_trie(EnchantPWL *pwl,
				    const char *const word, size_t len,
				    gboolean add_to_file);

static void enchant_pwl_check_cb(char* match,EnchantTrieMatcher* matcher);
static void enchant_pwl_suggest_cb(char* match,EnchantTrieMatcher* matcher);
static EnchantTrie* enchant_trie_init(void);
static void enchant_trie_free(EnchantTrie* trie);
static void enchant_trie_free_cb(void*,void*,void*);
static EnchantTrie* enchant_trie_insert(EnchantTrie* trie,const char *const word);
static void enchant_trie_find_matches(EnchantTrie* trie,EnchantTrieMatcher *matcher);
static void enchant_trie_find_matches_cb(void* keyV,void* subtrieV,void* matcherV);
static EnchantTrieMatcher* enchant_trie_matcher_init(const char* const word,
				int maxerrs,
		 		void(*cbfunc)(char*,EnchantTrieMatcher*),
				void* cbdata);
static void enchant_trie_matcher_free(EnchantTrieMatcher* matcher);
static void enchant_trie_matcher_pushpath(EnchantTrieMatcher* matcher,char* newchars);
static void enchant_trie_matcher_poppath(EnchantTrieMatcher* matcher,int num);

static int edit_dist(const char* word1, const char* word2);
static char* soundex(const char* word);

static void
enchant_lock_file (FILE * f)
{
#if defined(HAVE_FLOCK)
	flock (fileno (f), LOCK_EX);
#elif defined(HAVE_LOCKF)
	lockf (fileno (f), F_LOCK, 0);
#else
	/* TODO: win32, UNIX fcntl. This race condition probably isn't too bad. */
#endif /* HAVE_FLOCK */
}

static void
enchant_unlock_file (FILE * f)
{
#if defined(HAVE_FLOCK)
	flock (fileno (f), LOCK_UN);
#elif defined(HAVE_LOCKF)
	lockf (fileno (f), F_ULOCK, 0);
#else
	/* TODO: win32, UNIX fcntl. This race condition probably isn't too bad. */
#endif /* HAVE_FLOCK */
}

/**
 * enchant_pwl_init
 *
 * Returns: a new PWL object used to store/check/suggest words.
 */
EnchantPWL* enchant_pwl_init(void)
{
	EnchantPWL *pwl;

	pwl = g_new0(EnchantPWL, 1);
	pwl->words_in_trie = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	return pwl;
}

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

/**
 * enchant_pwl_init
 *
 * Returns: a new PWL object used to store/check/suggest words.
 */ 
EnchantPWL* enchant_pwl_init_with_file(const char * file)
{
	FILE *f;

	f = fopen (file, "r");
	if (f) 
		{
			EnchantPWL *pwl;
			char line[BUFSIZ];

			pwl = enchant_pwl_init();
			pwl->filename = g_strdup(file);

			enchant_lock_file (f);
			
			while (NULL != (fgets (line, sizeof (line), f)))
				{
					size_t l = strlen(line)-1;
					if (line[l]=='\n') 
						line[l] = '\0';
					
					enchant_pwl_add_to_trie(pwl, line, strlen(line), FALSE);
				}
			
			enchant_unlock_file (f);
			fclose (f);

			return pwl;
		} 

	return NULL;
}

void enchant_pwl_free(EnchantPWL *pwl)
{
	enchant_trie_free(pwl->trie);
	g_free(pwl->filename);
	g_hash_table_destroy (pwl->words_in_trie);
	g_free(pwl);
}

static void enchant_pwl_add_to_trie(EnchantPWL *pwl,
				    const char *const word, size_t len,
				    gboolean add_to_file)
{
	char * case_folded_word;

	case_folded_word = g_utf8_casefold (word, len);
	if(NULL != g_hash_table_lookup (pwl->words_in_trie, case_folded_word)) {
		g_free (case_folded_word);
		return;
	}
	
	g_hash_table_insert (pwl->words_in_trie, case_folded_word, GINT_TO_POINTER(1));

	pwl->trie = enchant_trie_insert(pwl->trie, case_folded_word);

	if (add_to_file && (pwl->filename != NULL))
		{
			FILE *f;
			
			f = fopen(pwl->filename, "a");
			if (f)
				{
					enchant_lock_file (f);
					fwrite (word, sizeof(char), len, f);
					fwrite ("\n", sizeof(char), 1, f);
					enchant_unlock_file (f);
					fclose (f);
				}	
		}
}

void enchant_pwl_add(EnchantPWL *pwl,
		     const char *const word, size_t len)
{
	enchant_pwl_add_to_trie(pwl, word, len, TRUE);
}

int enchant_pwl_check(EnchantPWL *pwl, const char *const word, size_t len)
{
	EnchantTrieMatcher* matcher;
	char * case_folded_word;
	int count = 0;
	
	case_folded_word = g_utf8_casefold (word, len);

	matcher = enchant_trie_matcher_init(case_folded_word,0,enchant_pwl_check_cb,
					    &count);
	enchant_trie_find_matches(pwl->trie,matcher);
	enchant_trie_matcher_free(matcher);
	g_free(case_folded_word);

	return (count != 0 ? 0 : 1);
}

static void enchant_pwl_check_cb(char* match,EnchantTrieMatcher* matcher)
{
	g_free(match);
	(*((int*)(matcher->cbdata)))++;
}

char** enchant_pwl_suggest(EnchantPWL *pwl,const char *const word,
			   size_t len, size_t* out_n_suggs)
{
	EnchantTrieMatcher* matcher;
	EnchantSuggList sugg_list;
	char *case_folded_word;

	sugg_list.suggs = g_new0(char*,ENCHANT_PWL_MAX_SUGGS+1);
	sugg_list.sugg_errs = g_new0(int,ENCHANT_PWL_MAX_SUGGS);
	sugg_list.n_suggs = 0;

	case_folded_word = g_utf8_casefold (word, len);
	matcher = enchant_trie_matcher_init(case_folded_word,ENCHANT_PWL_MAX_ERRORS,
					    enchant_pwl_suggest_cb,
					    &sugg_list);
	enchant_trie_find_matches(pwl->trie,matcher);
	enchant_trie_matcher_free(matcher);
	g_free(case_folded_word);

	g_free(sugg_list.sugg_errs);
	sugg_list.suggs[sugg_list.n_suggs] = NULL;
	(*out_n_suggs) = sugg_list.n_suggs;
	
	return sugg_list.suggs;
}

static void enchant_pwl_suggest_cb(char* match,EnchantTrieMatcher* matcher)
{
	EnchantSuggList* sugg_list;
	size_t loc, i, shuffleTo;
	int changes = 0;  /* num words added to list */

	sugg_list = (EnchantSuggList*)(matcher->cbdata);

	/* Find appropriate location in the array, if any */
	/* In future, this could be done using binary search...  */
	for(loc=0; loc < sugg_list->n_suggs; loc++) {
		/* Better than an existing suggestion, so stop */
		if(sugg_list->sugg_errs[loc] > matcher->num_errors) {
			break;
		}
		/* Already in the list with better score, just return */
		if(strcmp(match,sugg_list->suggs[loc])==0) {
			g_free(match);
			return;
		}
	}
	/* If it's not going to fit, just throw it away */
	if(loc >= ENCHANT_PWL_MAX_SUGGS) {
		g_free(match);
		return;
	}

	changes++;
	
	/* Find the location to shuffle other elements up to.
	 * If the new word already exists, delete it and stuffle up to there
	 * Otherwise, if we reach max suggs, delete last one
	 * Otherwise, shuffle up to end of list
	 */
	for(i=loc; i < sugg_list->n_suggs; i++){
		if(strcmp(match,sugg_list->suggs[i]) == 0) {
			g_free(sugg_list->suggs[i]);
			changes--;
			break;
		}
	}
	if(i == ENCHANT_PWL_MAX_SUGGS) {
		i--;
		changes--;
		g_free(sugg_list->suggs[i]);
	}
	shuffleTo = i;

	/* Shuffle other entries along to make space for new one */
	for(i=shuffleTo; i > loc; i--) {
		sugg_list->suggs[i] = sugg_list->suggs[i-1];
		sugg_list->sugg_errs[i] = sugg_list->sugg_errs[i-1];
	}

	sugg_list->suggs[loc] = match;
	sugg_list->sugg_errs[loc] = matcher->num_errors;
	sugg_list->n_suggs = sugg_list->n_suggs + changes;

}

void enchant_pwl_free_string_list(EnchantPWL *pwl, char** string_list)
{
	g_strfreev(string_list);
}

static EnchantTrie* enchant_trie_init(void)
{
	EnchantTrie* trie;

	trie = g_new(EnchantTrie,1);
	trie->value = NULL;
	trie->subtries = NULL;

	return trie;
}

static void enchant_trie_free(EnchantTrie* trie)
{
	/* Dont ever free NULL, or the EOSTrie pointer */
	if(trie == NULL || trie == EOSTrie) {
		return;
	}

	/* Because we have not set a destroy function for the hashtable
 	 * (to make code cleaner below), we need to explicitly free all
 	 * subtries with a callback function.
 	 */
	if (trie->subtries != NULL) {
		g_hash_table_foreach(trie->subtries,enchant_trie_free_cb,NULL);
		g_hash_table_destroy(trie->subtries);
	}

	if (trie->value != NULL) {
		g_free(trie->value);
	}

	g_free(trie);
}

static void enchant_trie_free_cb(void* key, void* value, void* data)
{
	enchant_trie_free((EnchantTrie*) value);
}

static EnchantTrie* enchant_trie_insert(EnchantTrie* trie,const char *const word)
{
	char *tmpWord;
	ssize_t nxtCh = 0;
	EnchantTrie* subtrie;

	if (trie == NULL) {
		trie = enchant_trie_init();
	}

	if (trie->value == NULL) {
		if (trie->subtries == NULL) {
			/*  When single word, store in trie->value */
			trie->value = g_strdup(word);
		} else {
			/* Store multiple words in subtries */
			if (word[0] == '\0') {
				/* Mark end-of-string with special node */
				tmpWord = g_strdup("");
				g_hash_table_insert(trie->subtries,
							tmpWord,EOSTrie);
			} else {
				nxtCh = (ssize_t)(g_utf8_next_char(word)-word);
				tmpWord = g_strndup(word,nxtCh);
				subtrie = g_hash_table_lookup(trie->subtries,
								tmpWord);
                        	subtrie = enchant_trie_insert(subtrie,
								(word+nxtCh));
				g_hash_table_insert(trie->subtries,
							tmpWord,subtrie);
			}
		}
	} else {
		/* Create new hastable for subtries, and reinsert */
		trie->subtries = g_hash_table_new_full(g_str_hash, 
					g_str_equal,g_free, NULL);
		tmpWord = trie->value;
		trie->value = NULL;
		enchant_trie_insert(trie,tmpWord);
		enchant_trie_insert(trie,word);
		g_free(tmpWord);
	}

	return trie;
}

static void enchant_trie_find_matches(EnchantTrie* trie,EnchantTrieMatcher *matcher)
{
	int errs = 0;
	ssize_t nxtChI = 0, oldPos = 0;
	char* nxtChS = NULL;
	EnchantTrie* subtrie = NULL;

	g_return_if_fail(matcher);

	/* Cant match in the empty trie */
	if(trie == NULL) {
		return;
	}

	/* Bail out if over the error limits */
	if(matcher->num_errors > matcher->max_errors){
		return;
	}

	/* If the end of a string has been reached, no point recursing */
	if (trie == EOSTrie) {
		errs = matcher->num_errors;
		matcher->num_errors = errs + strlen(matcher->word) \
						- matcher->word_pos;
		if (matcher->num_errors <= matcher->max_errors) {
			matcher->cbfunc(g_strdup(matcher->path),matcher);
		}
		matcher->num_errors = errs;
		return;
	}

	/* If there is a value, just check it, no recursion */
	if (trie->value != NULL) {
		errs = matcher->num_errors;
		matcher->num_errors = errs + edit_dist(trie->value,
					&(matcher->word[matcher->word_pos]));
		if (matcher->num_errors <= matcher->max_errors) {
			matcher->cbfunc(g_strconcat(matcher->path,
							trie->value,NULL),
					matcher);
		}
		matcher->num_errors = errs;
		return;
	}

	nxtChI = (ssize_t)(g_utf8_next_char(&matcher->word[matcher->word_pos]) - matcher->word);
	nxtChS = g_strndup(&matcher->word[matcher->word_pos],
			(nxtChI - matcher->word_pos));

	/* Precisely match the first character, and recurse */
	subtrie = g_hash_table_lookup(trie->subtries,nxtChS);
	if (subtrie != NULL) {
		enchant_trie_matcher_pushpath(matcher,nxtChS);
		oldPos = matcher->word_pos;
		matcher->word_pos = nxtChI;
		enchant_trie_find_matches(subtrie,matcher);
		matcher->word_pos = oldPos;
		enchant_trie_matcher_poppath(matcher,strlen(nxtChS));
	}

	g_free(nxtChS);

	if (matcher->word[matcher->word_pos] != '\0') {
		matcher->num_errors++;
		/* Match on inserting word[0] */
		oldPos = matcher->word_pos;
		matcher->word_pos = nxtChI;
		enchant_trie_find_matches(trie,matcher);
		matcher->word_pos = oldPos;
		/* for each subtrie, match on delete or swap word[0] */
		g_hash_table_foreach(trie->subtries,
					enchant_trie_find_matches_cb,
					matcher);
		matcher->num_errors--;
	}
}

static void enchant_trie_find_matches_cb(void* keyV,void* subtrieV,void* matcherV)
{
	char* key;
	EnchantTrie* subtrie;
	EnchantTrieMatcher* matcher;
	ssize_t nxtChI, oldPos;

	key = (char*) keyV;
	subtrie = (EnchantTrie*) subtrieV;
	matcher = (EnchantTrieMatcher*) matcherV;

	nxtChI = (ssize_t) (g_utf8_next_char(&matcher->word[matcher->word_pos]) - matcher->word);

	/* Dont handle actual matches, that's already done */
	if (strncmp(key,&matcher->word[matcher->word_pos],nxtChI-matcher->word_pos) == 0) {
		return;
	}

	enchant_trie_matcher_pushpath(matcher,key);

	/* Match on deleting word[0] */
	enchant_trie_find_matches(subtrie,matcher);
	/* Match on transposing word[0] */
	oldPos = matcher->word_pos;
	matcher->word_pos = nxtChI;
	enchant_trie_find_matches(subtrie,matcher);
	matcher->word_pos = oldPos;

	enchant_trie_matcher_poppath(matcher,strlen(key));
}

static EnchantTrieMatcher* enchant_trie_matcher_init(const char* const word,
				int maxerrs,
		 		void(*cbfunc)(char*,EnchantTrieMatcher*),
				void* cbdata)
{
	EnchantTrieMatcher* matcher;

	matcher = g_new(EnchantTrieMatcher,1);
	matcher->num_errors = 0;
	matcher->max_errors = maxerrs;
	matcher->word = word;
	matcher->word_pos = 0;
	matcher->path = g_new0(char,10);
	matcher->path[0] = '\0';
	matcher->path_len = 10;
	matcher->path_pos = 0;
	matcher->cbfunc = cbfunc;
	matcher->cbdata = cbdata;

	return matcher;
}

static void enchant_trie_matcher_free(EnchantTrieMatcher* matcher)
{
	g_free(matcher->path);
	g_free(matcher);
}

static void enchant_trie_matcher_pushpath(EnchantTrieMatcher* matcher,char* newchars)
{
	ssize_t len, i;

	len = strlen(newchars);
	if(matcher->path_pos + len >= matcher->path_len) {
		matcher->path_len = matcher->path_len + len + 10;
		matcher->path = g_renew(char,matcher->path,matcher->path_len);
	}

	for(i = 0; i < len; i++) {
		matcher->path[matcher->path_pos + i] = newchars[i];
	}
	matcher->path_pos = matcher->path_pos + len;
	matcher->path[matcher->path_pos] = '\0';
}

static void enchant_trie_matcher_poppath(EnchantTrieMatcher* matcher,int num)
{
	g_return_if_fail(matcher->path_pos >= 0);
	matcher->path_pos = matcher->path_pos - num;
	if(matcher->path_pos < 0) {
		matcher->path_pos = 0;
	}
	matcher->path[matcher->path_pos] = '\0';
}

static int edit_dist(const char* word1, const char* word2)
{
	int len1, len2, cost, i, j;
	int v1, v2, v3;
	int* table;

	len1 = strlen(word1);
	len2 = strlen(word2);

	table = g_new0(int,(len1+1)*(len2+1));
	
	/* Initialise outer rows of table */
	for (i=0; i < len1 + 1; i++) {
		table[i*(len2+1)] = i;
	}
	for (j=0; j < len2 + 1; j++) {
		table[j] = j;
	}

	/* Fill in table in dynamic programming style */
	for (i=1; i < len1+1; i++){
		for(j=1; j < len2+1; j++) {
			if(word1[i-1] == word2[j-1]) {
				cost = 0;
			} else {
				cost = 1;
			}
			v1 = table[(i-1)*(len2+1)+j] + 1;
			v2 = table[i*(len2+1)+(j-1)] + 1;
			v3 = table[(i-1)*(len2+1)+(j-1)] + cost;
			if (v1 < v2 && v1 < v3) {
				cost = v1;
			} else if (v2 < v3) {
				cost = v2;
			} else {
				cost = v3;
			}
			table[i*(len2+1)+j] = cost;
		}
	}

	cost = table[len1*(len2+1) + len2];
	g_free(table);
	return cost;
}

