#include "hashmgr.hxx"
#include "affixmgr.hxx"
#include "suggestmgr.hxx"
#include "csutil.hxx"
#include "langnum.hxx"

#define NOCAP   0
#define INITCAP 1
#define ALLCAP  2
#define HUHCAP  3
#define HUHINITCAP  4

#define MAXSUGGESTION 15
#define MAXSHARPS 5

#ifdef W32
#define DLLTEST2_API __declspec(dllexport)
#endif

#ifndef _MYSPELLMGR_HXX_
#define _MYSPELLMGR_HXX_

#ifdef W32
class DLLTEST2_API Hunspell
#else
class Hunspell
#endif
{
  AffixMgr*       pAMgr;
  HashMgr*        pHMgr;
  SuggestMgr*     pSMgr;
  char *          encoding;
  struct cs_info * csconv;
  struct unicode_info2 * utfconv;
  int             langnum;
  int             utf8;
  int             complexprefixes;
  char**          wordbreak;

/* XXX not stateless variables for compound handling */
  char *	  prevroot;
  int             prevcompound;

/* forbidden_compound:
 * 0 = not forbidden
 * 1 = forbidden
 * 2 = forbidden compound (written without dash in Hungarian)
 */
  int		  forbidden_compound;
  

public:

  /* Hunspell(aff, dic) - constructor of Hunspell class
   * input: path of affix file and dictionary file
   */
  
  Hunspell(const char * affpath, const char * dpath);

  ~Hunspell();

  /* spell(word) - spellcheck word
   * output: 0 = bad word, not 0 = good word
   */
   
  int spell(const char *);

  /* suggest(suggestions, word) - search suggestions
   * input: pointer to an array of strings pointer and the (bad) word
   *   array of strings pointer (here *slst) may not be initialized
   * output: number of suggestions in string array, and suggestions in
   *   a newly allocated array of strings (*slts will be NULL when number
   *   of suggestion equals 0.)
   */

  int suggest(char*** slst, const char * word);

  /* handling custom dictionary */

  int put_word(const char * word);

  /* suffix is an affix flag string, similarly in dictionary files */
  
  int put_word_suffix(const char * word, const char * suffix);
  
  /* pattern is a sample dictionary word 
   * put word into custom dictionary with affix flags of pattern word
   */
  
  int put_word_pattern(const char * word, const char * pattern);

  /* other */

  char * get_dic_encoding();
  const char * get_wordchars();
  unsigned short * get_wordchars_utf16(int * len);
  struct cs_info * get_csconv();
  struct unicode_info2 * get_utf_conv();
  const char * get_version();

  /* experimental functions */

  /* morphological analysis */
  
  char * morph(const char * word);
  int analyze(char*** out, const char *word);

  char * morph_with_correction(const char * word);

  /* stemmer function */
  
  int stem(char*** slst, const char * word);

  /* spec. suggestions */
  int suggest_auto(char*** slst, const char * word);
  int suggest_pos_stems(char*** slst, const char * word);
  char * get_possible_root();

  /* not threadsafe functions for Hunspell command line API */
  
  char * get_prevroot();
  int get_prevcompound();
  int get_forbidden_compound();

private:
   int    cleanword(char *, const char *, int * pcaptype, int * pabbrev);
   int    cleanword2(char *, const char *, w_char *, int * w_len, int * pcaptype, int * pabbrev);
   void   mkinitcap(char *);
   int    mkinitcap2(char * p, w_char * u, int nc);
   int    mkinitsmall2(char * p, w_char * u, int nc);
   void   mkallcap(char *);
   int    mkallcap2(char * p, w_char * u, int nc);
   void   mkallsmall(char *);
   int    mkallsmall2(char * p, w_char * u, int nc);
   struct hentry * check(const char *);
   char * sharps_u8_l1(char * dest, char * source);
   hentry * spellsharps(char * base, char *, int, int, char * tmp);
   int    is_keepcase(const hentry * rv);
   int    insert_sug(char ***slst, char * word, int *ns);

};

#endif
