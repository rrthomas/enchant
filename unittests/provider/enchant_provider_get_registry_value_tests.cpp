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
#include "../EnchantTestFixture.h"

struct EnchantGetRegistryValue_TestFixture : EnchantTestFixture{
    //Teardown
    ~EnchantGetRegistryValue_TestFixture(){
        ClearRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Test", L"Value");
        ClearRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Test", L"Value");
    }

    void SetUserRegistryValue(const std::string& value)
    {
        SetRegistryValue(HKEY_CURRENT_USER, L"Software\\Enchant\\Test", L"Value", value);
    }

    void SetMachineRegistryValue(const std::string& value)
    {
        SetRegistryValue(HKEY_LOCAL_MACHINE, L"Software\\Enchant\\Test", L"Value", value);
    }
};


/**
 * enchant_get_registry_value
 * @prefix: Your category, such as "Ispell" or "Myspell"
 * @key: The tag within your category that you're interested in
 *
 * Returns: the value if it exists and is not an empty string ("") or %null otherwise. Must be free'd.
 *
 * Choices:    User         Machine        Result
 *             "hello"      "world"        "hello"
 *             "hello"      NULL           "hello"
 *             "hello"      ""             "hello"
 *             ""           "world"        "world"
 *             ""           NULL           ""
 *             ""           ""             ""
 *             NULL         "world"        "world"
 *             NULL         NULL           NULL
 *             NULL         ""             ""
 *
 * This API is private to the providers.
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
#ifdef _WIN32
TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_UserAndMachine_ValueFromUser)
{
  SetUserRegistryValue("hello");
  SetMachineRegistryValue("world");

  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(value);
  CHECK_EQUAL("hello", value);

  g_free(value);
}

TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_UserOnly_ValueFromUser)
{
  SetUserRegistryValue("hello");
  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(value);
  CHECK_EQUAL("hello", value);

  g_free(value);
}

TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_MachineOnly_ValueFromMachine)
{
  SetMachineRegistryValue("world");
  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(value);
  CHECK_EQUAL("world", value);

  g_free(value);
}

TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_UserEmptyAndMachineSet_ValueFromMachine)
{
  SetUserRegistryValue("");
  SetMachineRegistryValue("world");

  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(value);
  CHECK_EQUAL("world", value);

  g_free(value);
}

TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_UserEmptyAndMachineNotSet_Null)
{
  SetUserRegistryValue("");

  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(!value);

  g_free(value);
}
#endif


TEST(GetRegistryValue_None_NULL)
{
  char * value = enchant_get_registry_value("Test", "Value");

  CHECK(value == NULL);
}


/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST(GetRegistryValue_NullPrefix_NULL)
{
  char * value = enchant_get_registry_value(NULL, "Value");

  CHECK(value == NULL);
}

TEST(GetRegistryValue_NullKey_NULL)
{
  char * value = enchant_get_registry_value("Test", NULL);

  CHECK(value == NULL);
}

#ifdef _WIN32
TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_NullPrefix_Null)
{
  SetUserRegistryValue("hello");

  char * value = enchant_get_registry_value(NULL, "Value");

  CHECK(value == NULL);
}

TEST_FIXTURE(EnchantGetRegistryValue_TestFixture, 
             GetRegistryValue_NullKey_Null)
{
  SetUserRegistryValue("hello");

  char * value = enchant_get_registry_value("Test", NULL);

  CHECK(value == NULL);
}

#endif