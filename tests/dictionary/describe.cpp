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

static bool callbackCalled;

void EnchantSingleDictionaryDescribeCallback (const char * const lang_tag,
				                          const char * const provider_name,
				                          const char * const provider_desc,
				                          const char * const provider_file,
				                          void * user_data)
{
    DictionaryDescription* description = reinterpret_cast<DictionaryDescription*>(user_data);
    *description = DictionaryDescription(lang_tag, provider_name, provider_desc, provider_file);
    callbackCalled = true;
}

static void * global_user_data;
void EnchantSingleDictionaryDescribeAssignUserDataToStaticCallback (const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              const char * const,
				                                              void * user_data)
{
    global_user_data = user_data;
    callbackCalled = true;
}

struct EnchantDictionaryDescribe_TestFixtureBase : EnchantDictionaryTestFixture
{
    //Setup
    EnchantDictionaryDescribe_TestFixtureBase(ConfigureHook userConfig, const std::string& languageTag="qaa"):
            EnchantDictionaryTestFixture(userConfig, languageTag)
    { 
        callbackCalled = false;
    }
    DictionaryDescription _description;
};

struct EnchantDictionaryDescribe_TestFixture_qaa : EnchantDictionaryDescribe_TestFixtureBase
{
    //Setup
    EnchantDictionaryDescribe_TestFixture_qaa():
            EnchantDictionaryDescribe_TestFixtureBase(EmptyDictionary_ProviderConfiguration, "qaa")
    { }
};

struct EnchantDictionaryDescribe_TestFixture_qaaqaa : EnchantDictionaryDescribe_TestFixtureBase
{
    //Setup
    EnchantDictionaryDescribe_TestFixture_qaaqaa():
            EnchantDictionaryDescribe_TestFixtureBase(EmptyDictionary_ProviderConfiguration, "qaa,qaa")
    { }
};

#define EnchantDictionaryDescribe_TestFixture EnchantDictionaryDescribe_TestFixture_qaa
#include "describe.i"

#undef EnchantDictionaryDescribe_TestFixture
#define EnchantDictionaryDescribe_TestFixture EnchantDictionaryDescribe_TestFixture_qaaqaa
#include "describe.i"
