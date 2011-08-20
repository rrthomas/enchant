/* string replacement list class */
#ifndef _REPLIST_HXX_
#define _REPLIST_HXX_

#include "hunvisapi.h"

#include "w_char.hxx"

class LIBHUNSPELL_DLL_EXPORTED RepList
{
protected:
    replentry ** dat;
    int size;
    int pos;

public:
    RepList(int n=0);
    virtual ~RepList();

    int get_pos();
    virtual int add(char * pat1, char * pat2);
    replentry * item(int n);
    virtual int near(const char * word);
    virtual int match(const char * word, int n);
    virtual int conv(const char * word, char * dest);
};
#endif
