/* Copyright (c) 2007 Eric Scott Albright
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <UnitTest++/UnitTest++.h>
#include <enchant.h>
#include <algorithm>

#include "EnchantDictionaryTestFixture.h"

static bool dictCheckCalled;

static int
MockDictionaryCheck (EnchantDict * dict, const char *const word, size_t len)
{
    dict;
    dictCheckCalled = true;
    if(strncmp("hello", word, len)==0)
    {
        return 0; //good word
    }
    return 1; // bad word
}


static EnchantDict* MockProviderRequestCheckMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestBasicMockDictionary(me, tag);
    dict->check = MockDictionaryCheck;
    return dict;
}

static void DictionaryCheck_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestCheckMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryRemoveFromSession_TestFixture_qaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryRemoveFromSession_TestFixture_qaa():
            EnchantDictionaryTestFixture(DictionaryCheck_ProviderConfiguration, "qaa")
    { 
        dictCheckCalled = false;
    }
};

struct EnchantDictionaryRemoveFromSession_TestFixture_qaaqaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryRemoveFromSession_TestFixture_qaaqaa():
            EnchantDictionaryTestFixture(DictionaryCheck_ProviderConfiguration, "qaa,qaa")
    { 
        dictCheckCalled = false;
    }
};

#define EnchantDictionaryRemoveFromSession_TestFixture EnchantDictionaryRemoveFromSession_TestFixture_qaa
#include "remove_from_session.i"

#undef EnchantDictionaryRemoveFromSession_TestFixture
#define EnchantDictionaryRemoveFromSession_TestFixture EnchantDictionaryRemoveFromSession_TestFixture_qaaqaa
#include "remove_from_session.i"
