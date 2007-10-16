#ifndef _HTYPES_HXX_
#define _HTYPES_HXX_

#define MAXDELEN    8192

#define ROTATE_LEN   5

#define ROTATE(v,q) \
   (v) = ((v) << (q)) | (((v) >> (32 - q)) & ((1 << (q))-1));

// approx. number  of user defined words
#define USERWORD 1000

struct hentry
{
  unsigned char blen; // word length in bytes
  unsigned char clen; // word length in characters (different for UTF-8 enc.)
  short    alen;      // length of affix flag vector
  unsigned short * astr;  // affix flag vector
  struct   hentry * next; // next word with same hash code
  struct   hentry * next_homonym; // next homonym word (with same hash code)
#ifdef HUNSPELL_EXPERIMENTAL
  char *   description; // morphological data (optional)
#endif
  char     var;       // variable fields (only for special pronounciation yet)
  char     word;      // variable-length word (8-bit or UTF-8 encoding)
};

#endif
