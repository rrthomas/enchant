#ifndef _SUGGESTMGR_HXX_
#define _SUGGESTMGR_HXX_

#define MAXSWL 100

#include "atypes.hxx"
#include "affixmgr.hxx"

class SuggestMgr
{
  char *          ctry;
  int             ctryl;
  AffixMgr*       pAMgr;
  int             maxSug;
 
public:
  SuggestMgr(const char * tryme, int maxn, AffixMgr *aptr);
  ~SuggestMgr();

  int suggest(char*** slst, const char * word);
  int check(const char *, int);

private:
   int replchars(char**, const char *, int);
   int forgotchar(char **, const char *, int);
   int swapchar(char **, const char *, int);
   int extrachar(char **, const char *, int);
   int badchar(char **, const char *, int);
   int twowords(char **, const char *, int);
};

#endif

