#ifndef _AFFIXMGR_HXX_
#define _AFFIXMGR_HXX_

#include "atypes.hxx"
#include "baseaffix.hxx"
#include "hashmgr.hxx"
#include <cstdio>

class AffixMgr
{

  AffEntry *          pStart[SETSIZE];
  AffEntry *          sStart[SETSIZE];
  HashMgr *           pHMgr;
  char *              trystring;
  char *              encoding;
  char *              compound;
  int                 cpdmin;
  int                 numrep;
  replentry *         reptable;

public:
 
  AffixMgr(const char * affpath, HashMgr * ptr);
  ~AffixMgr();
  struct hentry *     affix_check(const char * word, int len);
  struct hentry *     prefix_check(const char * word, int len);
  struct hentry *     suffix_check(const char * word, int len, int sfxopts, AffEntry* ppfx);
  struct hentry *     compound_check(const char * word, int len, char compound_flag);
  struct hentry *     lookup(const char * word);
  int                 get_numrep();
  struct replentry *  get_reptable();
  char *              get_encoding();
  char *              get_try_string();
  char *              get_compound();
             
private:
  int  parse_file(const char * affpath);
  int  parse_try(char * line);
  int  parse_set(char * line);
  int  parse_cpdflag(char * line);
  int  parse_cpdmin(char * line);
  int  parse_reptable(char * line, FILE * af);
  int  parse_affix(char * line, const char at, FILE * af);

  void encodeit(struct affentry * ptr, char * cs);
  int build_pfxlist(AffEntry* pfxptr);
  int build_sfxlist(AffEntry* sfxptr);
  int process_pfx_order();
  int process_sfx_order();
};

#endif

