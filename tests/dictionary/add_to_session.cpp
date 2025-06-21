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
#include "EnchantDictionaryTestFixture.h"

static bool addToSessionCalled;
static std::string wordToAdd;

static void
MockDictionaryAddToSession (EnchantDict * dict, const char *const word, size_t len)
{
    dict;
    addToSessionCalled = true;
    wordToAdd = std::string(word, len);
}

static EnchantDict* MockProviderRequestAddToSessionMockDictionary(EnchantProvider * me, const char *tag)
{
    
    EnchantDict* dict = MockProviderRequestBasicMockDictionary(me, tag);
    dict->add_to_session = MockDictionaryAddToSession;
    return dict;
}

static void DictionaryAddToSession_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->request_dict = MockProviderRequestAddToSessionMockDictionary;
     me->dispose_dict = MockProviderDisposeDictionary;
}

struct EnchantDictionaryAddToSession_TestFixture_qaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSession_TestFixture_qaa():
            EnchantDictionaryTestFixture(DictionaryAddToSession_ProviderConfiguration, "qaa")
    { 
        addToSessionCalled = false;
    }
};

struct EnchantDictionaryAddToSession_TestFixture_qaaqaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSession_TestFixture_qaaqaa():
            EnchantDictionaryTestFixture(DictionaryAddToSession_ProviderConfiguration, "qaa,qaa")
    { 
        addToSessionCalled = false;
    }
};

struct EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaa():
            EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, "qaa")
    { 
        addToSessionCalled = false;
    }
};

struct EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaaqaa : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaaqaa():
            EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, "qaa,qaa")
    { 
        addToSessionCalled = false;
    }
};

#define EnchantDictionaryAddToSession_TestFixture EnchantDictionaryAddToSession_TestFixture_qaa
#define EnchantDictionaryAddToSessionNotImplemented_TestFixture EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaa
#include "add_to_session.i"

#undef EnchantDictionaryAddToSession_TestFixture
#define EnchantDictionaryAddToSession_TestFixture EnchantDictionaryAddToSession_TestFixture_qaaqaa
#undef EnchantDictionaryAddToSessionNotImplemented_TestFixture
#define EnchantDictionaryAddToSessionNotImplemented_TestFixture EnchantDictionaryAddToSessionNotImplemented_TestFixture_qaaqaa
#include "add_to_session.i"
