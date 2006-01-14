#ifndef _HASHMGR_HXX_
#define _HASHMGR_HXX_

#include <cstdio>
#include "htypes.hxx"

enum flag { FLAG_CHAR, FLAG_LONG, FLAG_NUM, FLAG_UNI };

class HashMgr
{
  int             tablesize;
  struct hentry * tableptr;
  int	          userword;
  flag            flag_mode;
  int             complexprefixes;
  int             utf8;
  int                 numaliasf; // flag vector `compression' with aliases
  unsigned short **   aliasf;
  unsigned short *    aliasflen;
  int                 numaliasm; // morphological desciption `compression' with aliases
  char **             aliasm;


public:
  HashMgr(const char * tpath, const char * apath);
  ~HashMgr();

  struct hentry * lookup(const char *) const;
  int hash(const char *) const;
  struct hentry * walk_hashtable(int & col, struct hentry * hp) const;

  int put_word(const char * word, int wl, char * ap);
  int put_word_pattern(const char * word, int wl, const char * pattern);
  int decode_flags(unsigned short ** result, char * flags);
  unsigned short        decode_flag(const char * flag);
  char *                encode_flag(unsigned short flag);
  int is_aliasf();
  int is_aliasm();
  int get_aliasf(int index, unsigned short ** fvec);
  char * get_aliasm(int index);
  
private:
  int load_tables(const char * tpath);
  int add_word(const char * word, int wl, unsigned short * ap, int al, const char * desc);
  int load_config(const char * affpath);
  int parse_aliasf(char * line, FILE * af);
  int parse_aliasm(char * line, FILE * af);

};

#endif
