#ifndef _SUGGESTMGR_HXX_
#define _SUGGESTMGR_HXX_

#define MAXSWL 100
#define MAX_ROOTS 10
#define MAX_WORDS 500
#define MAX_GUESS 5

#include "atypes.hxx"
#include "affixmgr.hxx"
#include "hashmgr.hxx"

class SuggestMgr
{
  char *          ctry;
  int             ctryl;
  AffixMgr*       pAMgr;
  int             maxSug;
 
public:
  SuggestMgr(const char * tryme, int maxn, AffixMgr *aptr);
  ~SuggestMgr();

  int suggest(char** wlst, int ns, const char * word);
  int check(const char *, int);
  int ngsuggest(char ** wlst, char * word, HashMgr* pHMgr);

private:
   int replchars(char**, const char *, int);
   int forgotchar(char **, const char *, int);
   int swapchar(char **, const char *, int);
   int extrachar(char **, const char *, int);
   int badchar(char **, const char *, int);
   int twowords(char **, const char *, int);
   int ngram(int n, char * s1, const char * s2, bool uselen);
   void bubblesort( char ** rwd, int * rsc, int n);
};

#endif

