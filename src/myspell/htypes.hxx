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
  short    wlen;
  short    alen;
  char *   word;
  unsigned short * astr;
  struct   hentry * next;
  struct   hentry * next_homonym;
  char *   description;
};

#endif
