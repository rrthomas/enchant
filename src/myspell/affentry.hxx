#ifndef _AFFIX_HXX_
#define _AFFIX_HXX_

#include "atypes.hxx"
#include "baseaffix.hxx"
#include "affixmgr.hxx"


/* A Prefix Entry  */

class PfxEntry : public AffEntry
{
       AffixMgr*    pmyMgr;

       PfxEntry * next;
       PfxEntry * nexteq;
       PfxEntry * nextne;

public:

  PfxEntry(AffixMgr* pmgr, affentry* dp );
  ~PfxEntry();

  struct hentry *      check(const char * word, int len);

  inline unsigned char getFlag()   { return achar;   }
  inline const char *  getKey()    { return appnd;  } 

  inline PfxEntry *    getNext()   { return next;   }
  inline PfxEntry *    getNextNE() { return nextne; }
  inline PfxEntry *    getNextEQ() { return nexteq; }

  inline void   setNext(PfxEntry * ptr)   { next = ptr;   }
  inline void   setNextNE(PfxEntry * ptr) { nextne = ptr; }
  inline void   setNextEQ(PfxEntry * ptr) { nexteq = ptr; }
};




/* A Suffix Entry */

class SfxEntry : public AffEntry
{
       AffixMgr*    pmyMgr;
       char *       rappnd;

       SfxEntry *   next;
       SfxEntry *   nexteq;
       SfxEntry *   nextne;

public:

  SfxEntry(AffixMgr* pmgr, affentry* dp );
  ~SfxEntry();

  struct hentry *   check(const char * word, int len, int optflags, 
                                                       AffEntry* ppfx);

  inline unsigned char getFlag()   { return achar;   }
  inline const char *  getKey()    { return rappnd; } 

  inline SfxEntry *    getNext()   { return next;   }
  inline SfxEntry *    getNextNE() { return nextne; }
  inline SfxEntry *    getNextEQ() { return nexteq; }

  inline void   setNext(SfxEntry * ptr)   { next = ptr;   }
  inline void   setNextNE(SfxEntry * ptr) { nextne = ptr; }
  inline void   setNextEQ(SfxEntry * ptr) { nexteq = ptr; }

};


#endif


