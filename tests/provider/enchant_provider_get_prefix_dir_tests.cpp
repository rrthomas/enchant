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

#include <UnitTest++.h>
#include <enchant-provider.h>
#include <glib.h>
#include <string.h>
#include "EnchantTestFixture.h"

struct EnchantGetPrefixDirTestFixture : EnchantTestFixture{
  //Setup
  EnchantGetPrefixDirTestFixture()
  {
    expectedPrefixDir = ".";
  }

  const char* ExpectedPrefixDir()
  {
      return expectedPrefixDir.c_str();
  }

  std::string expectedPrefixDir;

};

/**
 * enchant_get_prefix_dir
 *
 * Returns a string giving the location of the base directory
 * of the enchant installation.
 *
 * This API is private to the providers.
 */

TEST_FIXTURE(EnchantGetPrefixDirTestFixture,
             EnchantGetPrefixDir)
{
    char* prefixDir = enchant_get_prefix_dir();
    const char* expectedPrefixDir = ExpectedPrefixDir();
    fprintf(stderr, "prefixDir: %s, expectedPrefixDir: %s\n", prefixDir, expectedPrefixDir);
    CHECK((expectedPrefixDir == NULL && prefixDir == NULL) ||
          strcmp(expectedPrefixDir, prefixDir) == 0);
    g_free(prefixDir);
}
