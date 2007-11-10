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

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST(EnchantGetUserLanguage)
{
    char* userLanguage = enchant_get_user_language();
    CHECK(userLanguage);
    g_free(userLanguage);
}

TEST(EnchantGetUserLanguage_FromLangEnvironmentVariable)
{
    std::string origLangEnv;
    bool hasLangEnv = (g_getenv("LANG") != NULL);
    if(hasLangEnv)
    {
        origLangEnv = std::string(g_getenv("LANG"));
    }

    g_setenv("LANG", "qaa", TRUE);
    char* userLanguage = enchant_get_user_language();
    CHECK(userLanguage);
    CHECK_EQUAL("qaa", userLanguage);

    g_free(userLanguage);

    if(hasLangEnv)
    {
        g_setenv("LANG", origLangEnv.c_str(), TRUE);
    }
    else{
        g_unsetenv("LANG");
    }
}

#ifndef _WIN32
TEST(EnchantGetUserLanguage_FromLocale)
{
    std::string origLocale(setlocale (LC_ALL, NULL));

    setlocale (LC_ALL, "qaa");
    char* userLanguage = enchant_get_user_language();
    CHECK(userLanguage);
    CHECK_EQUAL("qaa", userLanguage);

    g_free(userLanguage);

    setlocale (LC_ALL, origLocale.c_str());
}

TEST(EnchantGetUserLanguage_LocaleIsC_LocalIsEn)
{
    std::string origLocale(setlocale (LC_ALL, NULL));

    setlocale (LC_ALL, "C");
    char* userLanguage = enchant_get_user_language();
    CHECK(userLanguage);
    CHECK_EQUAL("en", userLanguage);

    g_free(userLanguage);

    setlocale (LC_ALL, origLocale.c_str());
}
#endif