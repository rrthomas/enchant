#include "license.readme"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "myspell.hxx"


MySpell::MySpell(const char * affpath, const char * dpath)
{
    encoding = NULL;
    csconv = NULL;

    /* first set up the hash manager */
    pHMgr = new HashMgr(dpath);

    /* next set up the affix manager */
    /* it needs access to the hash manager lookup methods */
    pAMgr = new AffixMgr(affpath,pHMgr);

    /* get the preferred try string and the dictionary */
    /* encoding from the Affix Manager for that dictionary */
    char * try_string = pAMgr->get_try_string();
    encoding = pAMgr->get_encoding();
    csconv = get_current_cs(encoding);

    /* and finally set up the suggestion manager */
    pSMgr = new SuggestMgr(try_string, 15, pAMgr);
}


MySpell::~MySpell()
{
    if (pSMgr) delete pSMgr;
    if (pAMgr) delete pAMgr;
    if (pHMgr) delete pHMgr;
    pSMgr = NULL;
    pAMgr = NULL;
    pHMgr = NULL;
    csconv= NULL;
    if (encoding) free(encoding);
    encoding = NULL;
}


// make a copy of src at destination while removing all leading
// blanks and removing any trailing periods after recording
// their presence with the abbreviation flag
// also since already going through character by character, 
// set the capitalization type
// return the length of the "cleaned" word

int MySpell::cleanword(char * dest, const char * src, int * pcaptype, int * pabbrev)
{ 

  // with the new breakiterator code this should not be needed anymore
#if 0
   const char * special_chars = ".!_#$%&()* +,-/:;<=>?[]\\^`{|}~\t \x0a\x0d\x0ab\x0bb\x01\x0a1\'\"";
#else
   const char * special_chars = ".!_#$%&()* +,-/:;<=>?[]\\^`{|}~\t \x0a\x0d\x01\'\"";
#endif

   unsigned char * p = (unsigned char *) dest;
   const unsigned char * q = (const unsigned char * ) src;

   // first skip over any leading special characters
   while ((*q != '\0') && (strchr(special_chars,(int)(*q)))) q++;
   
   // now strip off any trailing special characters 
   // if a period comes after a normal char record its presence
   *pabbrev = 0;
   int nl = strlen((const char *)q);
   while ((nl > 0) && (strchr(special_chars,(int)(*(q+nl-1))))) {
       nl--;
   }
   if ( *(q+nl) == '.' ) *pabbrev = 1;
   
   // if no characters are left it can't be an abbreviation and can't be capitalized
   if (nl <= 0) { 
       *pcaptype = NOCAP;
       *pabbrev = 0;
       *p = '\0';
       return 0;
   }

   // now determine the capitalization type of the first nl letters
   int ncap = 0;
   int nneutral = 0;
   int nc = 0;
   while (nl > 0) {
       nc++;
       if (csconv[(*q)].ccase) ncap++;
       if (csconv[(*q)].cupper == csconv[(*q)].clower) nneutral++;
       *p++ = *q++;
       nl--;
   }
   // remember to terminate the destination string
   *p = '\0';

   // now finally set the captype
   if (ncap == 0) {
        *pcaptype = NOCAP;
   } else if ((ncap == 1) && csconv[(unsigned char)(*dest)].ccase) {
        *pcaptype = INITCAP;
  } else if ((ncap == nc) || ((ncap + nneutral) == nc)){
        *pcaptype = ALLCAP;
  } else {
        *pcaptype = HUHCAP;
  }
  return nc;
} 
       

int MySpell::spell(const char * word)
{
  char * rv=NULL;
  char cw[MAXWORDLEN+1];
  char wspace[MAXWORDLEN+1];

  int wl = strlen(word);
  if (wl > (MAXWORDLEN - 1)) return 0;
  int captype = 0;
  int abbv = 0;
  wl = cleanword(cw, word, &captype, &abbv);
  if (wl == 0) return 1;

  switch(captype) {
     case HUHCAP:
     case NOCAP:   { 
                     rv = check(cw); 
                     if ((abbv) && !(rv)) {
		         memcpy(wspace,cw,wl);
                         *(wspace+wl) = '.';
                         *(wspace+wl+1) = '\0';
                         rv = check(wspace);
                     }
                     break;
                   }

     case ALLCAP:
     case INITCAP: { 
                     memcpy(wspace,cw,(wl+1));
                     mkallsmall(wspace, csconv);
                     rv = check(wspace);
                     if (!rv) rv = check(cw);
                     if ((abbv) && !(rv)) {
		         memcpy(wspace,cw,wl);
                         *(wspace+wl) = '.';
                         *(wspace+wl+1) = '\0';
                         rv = check(wspace);
                     }
                     break; 
                   }
  }
  if (rv) return 1;
  return 0;
}


char * MySpell::check(const char * word)
{
  struct hentry * he = NULL;
  if (pHMgr)
     he = pHMgr->lookup (word);

  if ((he == NULL) && (pAMgr)) {
     // try stripping off affixes */
     he = pAMgr->affix_check(word, strlen(word));

     // try check compound word
     if ((he == NULL) && (pAMgr->get_compound())) {
          he = pAMgr->compound_check(word, strlen(word), (pAMgr->get_compound())[0]);
     }

  }

  if (he) return he->word;
  return NULL;
}



int MySpell::suggest(char*** slst, const char * word)
{
  char cw[MAXWORDLEN+1];
  char wspace[MAXWORDLEN+1];
  if (! pSMgr) return 0;
  int wl = strlen(word);
  if (wl > (MAXWORDLEN-1)) return 0;
  int captype = 0;
  int abbv = 0;
  wl = cleanword(cw, word, &captype, &abbv);
  if (wl == 0) return 0;
  int ns = 0;
  switch(captype) {
     case NOCAP:   { 
                     ns = pSMgr->suggest(slst, cw); 
                     break;
                   }

     case INITCAP: { 
                     memcpy(wspace,cw,(wl+1));
                     mkallsmall(wspace, csconv);
                     ns = pSMgr->suggest(slst, wspace);
                     for (int j=0; j < ns; j++)
                       mkinitcap((*slst)[j], csconv);
                     break;
                   }

     case HUHCAP: { 
                     ns = pSMgr->suggest(slst, cw);
                     if (ns == 0) {
                        memcpy(wspace,cw,(wl+1));
                        mkallsmall(wspace, csconv);
                        ns = pSMgr->suggest(slst, wspace);
		     }
                     break;
                   }

     case ALLCAP: { 
                     memcpy(wspace,cw,(wl+1));
                     mkallsmall(wspace, csconv);
                     ns = pSMgr->suggest(slst, wspace);
                     for (int j=0; j < ns; j++)
                       mkallcap((*slst)[j], csconv);
                     break;
                   }
  }
  return ns;
}


char * MySpell::get_dic_encoding()
{
  return encoding;
}

