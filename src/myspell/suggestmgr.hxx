#ifndef _SUGGESTMGR_HXX_
#define _SUGGESTMGR_HXX_

#define MAXSWL 100
#define MAXSWUTF8L (MAXSWL * 4)
#define MAX_ROOTS 50
#define MAX_WORDS 200
#define MAX_GUESS 200
#define MAXNGRAMSUGS 5

#define MINTIMER 500
#define MAXPLUSTIMER 500

#define NGRAM_IGNORE_LENGTH 0
#define NGRAM_LONGER_WORSE  1
#define NGRAM_ANY_MISMATCH  2

#include "atypes.hxx"
#include "affixmgr.hxx"
#include "hashmgr.hxx"
#include "langnum.hxx"
#include <time.h>

enum { LCS_UP, LCS_LEFT, LCS_UPLEFT };

class SuggestMgr
{
  char *          ctry;
  int             ctryl;
  w_char *        ctry_utf;

  AffixMgr*       pAMgr;
  int             maxSug;
  struct cs_info * csconv;
  struct unicode_info2 * utfconv;
  int             utf8;
  int             nosplitsugs;
  int             maxngramsugs;
  int             complexprefixes;


public:
  SuggestMgr(const char * tryme, int maxn, AffixMgr *aptr);
  ~SuggestMgr();

  int suggest(char*** slst, const char * word, int nsug);
  int ngsuggest(char ** wlst, char * word, HashMgr* pHMgr);
  int suggest_auto(char*** slst, const char * word, int nsug);
  int suggest_stems(char*** slst, const char * word, int nsug);
  int suggest_pos_stems(char*** slst, const char * word, int nsug);

  char * suggest_morph(const char * word);
  char * suggest_morph_for_spelling_error(const char * word);

private:
   int check(const char *, int, int, int *, time_t *);
   int check_forbidden(const char *, int);

   int replchars(char**, const char *, int, int);
   int doubledsyllable(char**, const char *, int, int);
   int forgotchar(char **, const char *, int, int);
   int swapchar(char **, const char *, int, int);
   int extrachar(char **, const char *, int, int);
   int badchar(char **, const char *, int, int);
   int twowords(char **, const char *, int, int);
   int fixstems(char **, const char *, int);

   int forgotchar_utf(char**, const w_char *, int wl, int, int);
   int extrachar_utf(char**, const w_char *, int wl, int, int);
   int badchar_utf(char **, const w_char *, int wl, int, int);
   int swapchar_utf(char **, const w_char *, int wl, int, int);

   int mapchars(char**, const char *, int, int);
   int map_related(const char *, int, char ** wlst, int, const mapentry*, int, int *, time_t *);
   int map_related_utf(w_char *, int, int, char ** wlst, int, const mapentry*, int, int *, time_t *);
   int ngram(int n, char * s1, const char * s2, int uselen);
   int mystrlen(const char * word);
   int equalfirstletter(char * s1, const char * s2);
   int commoncharacterpositions(char * s1, const char * s2, int * is_swap);
   void bubblesort( char ** rwd, int * rsc, int n);
   void lcs(const char * s, const char * s2, int * l1, int * l2, char ** result);
   int lcslen(const char * s, const char* s2);

};

#endif

