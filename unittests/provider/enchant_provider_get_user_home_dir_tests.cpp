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

#include "../EnchantTestFixture.h"
#include <UnitTest++.h>
#include <enchant-provider.h>

/**
 * enchant_get_user_home_dir
 *
 * Returns: the user's home directory, or %null. Returned value
 * must be free'd.
 *
 * This API is private to the providers.
 */

/*
 * The user's home directory on windows can be overridden using the registry
 * setting HKEY_CURRENT_USER\Software\Enchant\Config\Home_Dir
 */
#ifdef _WIN32
TEST_FIXTURE(EnchantTestFixture, 
             GetUserHomeDir_FromRegistry)
{
  std::string homeDir("here I am");
  SetRegistryHomeDir(homeDir);

  char * enchantUserHomeDir = enchant_get_user_home_dir();

  CHECK(enchantUserHomeDir);
  CHECK_EQUAL(homeDir, enchantUserHomeDir);

  g_free(enchantUserHomeDir);
}

#ifdef _WIN32
TEST_FIXTURE(EnchantTestFixture, 
             GetUserHomeDir_BlankFromRegistry_RegistryEntryIgnored)
{
  std::string homeDir("");
  SetRegistryHomeDir(homeDir);

  char * enchantUserHomeDir = enchant_get_user_home_dir();

  CHECK(enchantUserHomeDir);
  CHECK_EQUAL(g_get_home_dir(), enchantUserHomeDir);

  g_free(enchantUserHomeDir);
}
#endif

#endif

TEST_FIXTURE(EnchantTestFixture,
             GetUserHomeDir)
{
  char * enchantUserHomeDir = enchant_get_user_home_dir();

  CHECK(enchantUserHomeDir);
  CHECK_EQUAL(g_get_home_dir(), enchantUserHomeDir);

  g_free(enchantUserHomeDir);
}