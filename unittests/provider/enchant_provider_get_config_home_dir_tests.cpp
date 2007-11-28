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
 * enchant_get_user_config_dir
 *
 * Returns: the user's enchant directory, or %null. Returned value
 * must be free'd.
 *
 * The enchant directory is the place where enchant finds user providers and 
 * dictionaries and settings related to enchant
 *
 * This API is private to the providers.
 */

/*
 * The user's config directory on windows can be overridden using the registry
 * setting HKEY_CURRENT_USER\Software\Enchant\Config\Data_Dir
 */
#ifdef _WIN32
TEST_FIXTURE(EnchantTestFixture, 
             GetUserConfigDir_FromRegistry)
{
  std::string configDir("here I am");
  SetUserRegistryConfigDir(configDir);

  char * enchantUserConfigDir = enchant_get_user_config_dir();

  CHECK(enchantUserConfigDir);
  CHECK_EQUAL(configDir, enchantUserConfigDir);

  g_free(enchantUserConfigDir);
}

TEST_FIXTURE(EnchantTestFixture, 
             GetUserConfigDir_BlankFromRegistry_RegistryEntryIgnored)
{
  std::string configDir("");
  SetUserRegistryConfigDir(configDir);

  char * enchantUserConfigDir = enchant_get_user_config_dir();

  CHECK(enchantUserConfigDir);
  
  CHECK_EQUAL(GetEnchantHomeDirFromBase(g_get_user_config_dir()), enchantUserConfigDir);

  g_free(enchantUserConfigDir);
}
#endif

TEST_FIXTURE(EnchantTestFixture,
             GetUserConfigDir)
{
  char * enchantUserConfigDir = enchant_get_user_config_dir();

  CHECK(enchantUserConfigDir);
#ifdef _WIN32
  CHECK_EQUAL(GetEnchantHomeDirFromBase(g_get_user_config_dir()), enchantUserConfigDir);
#else
  CHECK_EQUAL(GetEnchantHomeDirFromBase(g_get_home_dir()), enchantUserConfigDir);
#endif
  g_free(enchantUserConfigDir);
}