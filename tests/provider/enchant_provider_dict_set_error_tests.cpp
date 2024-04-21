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
#include <enchant-provider.h>
#include "EnchantDictionaryTestFixture.h"

struct EnchantDictionarySetError_TestFixtureBase : EnchantDictionaryTestFixture
{
  //Setup
  EnchantDictionarySetError_TestFixtureBase(ConfigureHook userConfig, const std::string& languageTag="qaa"):
    EnchantDictionaryTestFixture(userConfig, languageTag)
  { }

  std::string GetErrorMessage(){
      const char* error = enchant_dict_get_error(_dict);
      if(error == NULL){
          return std::string();
      }
      return std::string(error);
  }
};

struct EnchantDictionarySetError_TestFixture_qaa : EnchantDictionarySetError_TestFixtureBase
{
    //Setup
    EnchantDictionarySetError_TestFixture_qaa():
            EnchantDictionarySetError_TestFixtureBase(EmptyDictionary_ProviderConfiguration, "qaa")
    { }
};

struct EnchantDictionarySetError_TestFixture_qaaqaa : EnchantDictionarySetError_TestFixtureBase
{
    //Setup
    EnchantDictionarySetError_TestFixture_qaaqaa():
            EnchantDictionarySetError_TestFixtureBase(EmptyDictionary_ProviderConfiguration, "qaa,qaa")
    { }
};

#define EnchantDictionarySetError_TestFixture EnchantDictionarySetError_TestFixture_qaa
#include "enchant_provider_dict_set_error_tests.i"

#undef EnchantDictionarySetError_TestFixture
#define EnchantDictionarySetError_TestFixture EnchantDictionarySetError_TestFixture_qaaqaa
#include "enchant_provider_dict_set_error_tests.i"
