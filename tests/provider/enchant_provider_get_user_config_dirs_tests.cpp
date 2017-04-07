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

#include "EnchantTestFixture.h"
#include <UnitTest++.h>
#include <enchant-provider.h>
#include <string.h>

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

TEST_FIXTURE(EnchantTestFixture,
             GetUserConfigDir)
{
  std::string enchantUserConfigDir = enchant_get_user_config_dir();
  std::string expected = getenv("ENCHANT_CONFIG_DIR");

  // FIXME: following crashes in UnitTest 1.4 with CHECK_EQUAL: use that when we can require >= 1.6
  CHECK(expected == enchantUserConfigDir);

  // Check it also works with ENCHANT_CONFIG_DIR unset
  g_unsetenv("ENCHANT_CONFIG_DIR");
  std::string enchantUserConfigDir2 = enchant_get_user_config_dir();
  char *expected2 = g_build_filename (g_get_user_config_dir(), "enchant", NULL);
  CHECK(strcmp(expected2, enchantUserConfigDir2.c_str()) == 0);

  // Restore env var
  g_setenv("ENCHANT_CONFIG_DIR", expected.c_str(), TRUE);
}
