#include "license.readme"

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <cstdio>

#include "hashmgr.hxx"

extern void mychomp(char * s);
extern char * mystrdup(const char *);


// build a hash table from a munched word list

HashMgr::HashMgr(const char * tpath)
{
  tablesize = 0;
  tableptr = NULL;
  int ec = load_tables(tpath);
  if (ec) {
    /* error condition - what should we do here */
    fprintf(stderr,"Hash Manager Error : %d\n",ec);
    fflush(stderr);
    if (tableptr) {
      free(tableptr);
    }
    tablesize = 0;
  }
}


HashMgr::~HashMgr()
{
  if (tableptr) {
    free(tableptr);
  }
  tablesize = 0;
}




// lookup a root word in the hashtable

struct hentry * HashMgr::lookup(const char *word)
{
    struct hentry * dp;
    if (tableptr) {
       dp = &tableptr[hash(word)];
       if (dp->word == NULL) return NULL;
       for (  ;  dp != NULL;  dp = dp->next) {
          if (strcmp(word,dp->word) == 0) return dp;
       }
    }
    return NULL;
}



// add a word to the hash table (private)

int HashMgr::add_word(const char * word, int wl, const char * aff, int al)
{
    struct hentry* hp = (struct hentry *) malloc (sizeof(struct hentry));
    hp->wlen = wl;
    hp->alen = al;
    hp->word = mystrdup(word);
    hp->astr = mystrdup(aff);
    hp->next = NULL;

    int i = hash(word);
    struct hentry * dp = &tableptr[i];
    
    if (dp->word == NULL) {
      *dp = *hp;
       free(hp);
    } else {
      while (dp->next != NULL) dp=dp->next; 
      dp->next = hp;
    }
    return 0;
}     



// load a munched word list and build a hash table on the fly

int HashMgr::load_tables(const char * tpath)
{
  int wl, al;
  char * ap;

  // raw dictionary - munched file
  FILE * rawdict = fopen(tpath, "r");
  if (rawdict == NULL) return 1;

  // first read the first line of file to get hash table size */
  char ts[MAXDELEN];
  if (! fgets(ts, MAXDELEN-1,rawdict)) return 2;
  mychomp(ts);
  tablesize = atoi(ts);
  tablesize = tablesize + 5;
  if ((tablesize %2) == 0) tablesize++;

  // allocate the hash table
  tableptr = (struct hentry *) calloc(tablesize, sizeof(struct hentry));
  if (! tableptr) return 3;
  for (int i=0; i<tablesize; i++) tableptr[i].word = NULL;

  // loop through all words on much list and add to hash
  // table and create word and affix strings

  while (fgets(ts,MAXDELEN-1,rawdict)) {
    mychomp(ts);
    // split each line into word and affix char strings
    ap = strchr(ts,'/');
    if (ap) {
      *ap = '\0';
      ap++;
      al = strlen(ap);
    } else {
      al = 0;
      ap = NULL;
    }

    wl = strlen(ts);

    // add the word and its index
    add_word(ts,wl,ap,al);

  }

  return 0;
}


// the hash function is a simple load and rotate
// algorithm borrowed

int HashMgr::hash(const char * word)
{
    long  hv = 0;
    for (int i=0; i < 4  &&  *word != 0; i++)
	hv = (hv << 8) | (*word++);
    while (*word != 0) {
      ROTATE(hv,ROTATE_LEN);
      hv ^= (*word++);
    }
    return (unsigned long) hv % tablesize;
}

