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
#include <enchant.h>
#include <algorithm>
#include <functional>
#include <vector>
#include "../EnchantBrokerTestFixture.h"

struct ProviderDescription 
{
    std::string Name;
    std::string Description;
    std::string DllFile;
    ProviderDescription(const char * const provider_name,
  	              const char * const provider_desc,
                  const char * const provider_dll_file):
        Name(provider_name), 
        Description(provider_desc), 
        DllFile(provider_dll_file)
    {}

    bool DataIsComplete() const
    {
        return (Name.length() &&
                Description.length() &&
                DllFile.length());
    }
};

void EnchantBrokerDescribeCallback (const char * const provider_name,
      	                            const char * const provider_desc,
		                            const char * const provider_dll_file,
		                            void * user_data)
{
    std::vector<ProviderDescription>* providerList = reinterpret_cast<std::vector<ProviderDescription>*>(user_data);

    providerList->push_back(ProviderDescription(provider_name, provider_desc, provider_dll_file));
}

static void * global_user_data;
void EnchantBrokerDescribeAssignUserDataToStaticCallback (const char * const,
      	                                    const char * const,
				                            const char * const,
				                            void * user_data)
{
    global_user_data = user_data;
}

struct EnchantBrokerNoProvidersTestFixture : EnchantTestFixture
{
    //Setup
    EnchantBrokerNoProvidersTestFixture()
    {
#ifdef _WIN32
        SetRegistryHomeDir("someplace_that_does_not_exist");
        SetUserRegistryModuleDir("someplace_that_does_not_exist");
        SetUserRegistryConfigDir("someplace_that_does_not_exist");
#endif
    _broker = enchant_broker_init ();
    }

    //Teardown
    ~EnchantBrokerNoProvidersTestFixture()
    {
        enchant_broker_free (_broker);
    }

    EnchantBroker* _broker;
    std::vector<ProviderDescription> _providerList;
};

static const char *
MockProvider2Identify (EnchantProvider *)
{
	return "mock2";
}

static void Provider2Configuration (EnchantProvider * me, const char *)
{
     me->identify = MockProvider2Identify;
     me->describe = MockProviderDescribe;
}

struct EnchantBrokerDescribeTestFixtureBase : EnchantBrokerTestFixture
{
    std::vector<ProviderDescription> _providerList;

    //Setup
    EnchantBrokerDescribeTestFixtureBase(ConfigureHook userConfiguration):
        EnchantBrokerTestFixture(userConfiguration, Provider2Configuration, true)
    { }
};

static void List_Providers_ProviderConfiguration (EnchantProvider * me, const char *)
{
     me->identify = MockProviderIdentify;
     me->describe = MockProviderDescribe;
}

struct EnchantBrokerDescribeTestFixture : EnchantBrokerDescribeTestFixtureBase
{
    //Setup
    EnchantBrokerDescribeTestFixture():
            EnchantBrokerDescribeTestFixtureBase(List_Providers_ProviderConfiguration)
    { 
        global_user_data = NULL;
    }
};


static void List_Providers_ProviderConfigurationNoIdentify (EnchantProvider * me, const char *)
{
     me->identify = NULL;
}

struct EnchantBrokerDescribe_ProviderLacksIdentify_TestFixture : EnchantBrokerDescribeTestFixtureBase
{
    //Setup
    EnchantBrokerDescribe_ProviderLacksIdentify_TestFixture():
            EnchantBrokerDescribeTestFixtureBase(List_Providers_ProviderConfigurationNoIdentify)
    {
        global_user_data = NULL;
    }
};

static void List_Providers_ProviderConfigurationNoDescribe (EnchantProvider * me, const char *)
{
     me->describe = NULL;
}

struct EnchantBrokerDescribe_ProviderLacksDescribe_TestFixture : EnchantBrokerDescribeTestFixtureBase
{
    //Setup
    EnchantBrokerDescribe_ProviderLacksDescribe_TestFixture():
            EnchantBrokerDescribeTestFixtureBase(List_Providers_ProviderConfigurationNoDescribe)
    {
        global_user_data = NULL;
    }
};

static const char *
MockProviderIllegalUtf8 (EnchantProvider *)
{
	return "\xa5\xf1\x08";
}

static void List_Providers_ProviderConfigurationInvalidIdentify (EnchantProvider * me, const char *)
{
     me->identify = MockProviderIllegalUtf8;
}

struct EnchantBrokerDescribe_ProviderHasInvalidUtf8Identify_TestFixture : EnchantBrokerDescribeTestFixtureBase
{
    //Setup
    EnchantBrokerDescribe_ProviderHasInvalidUtf8Identify_TestFixture():
            EnchantBrokerDescribeTestFixtureBase(List_Providers_ProviderConfigurationInvalidIdentify)
    {
        global_user_data = NULL;
    }
};

static void List_Providers_ProviderConfigurationInvalidDescribe (EnchantProvider * me, const char *)
{
     me->describe = MockProviderIllegalUtf8;
}

struct EnchantBrokerDescribe_ProviderHasInvalidUtf8Describe_TestFixture : EnchantBrokerDescribeTestFixtureBase
{
    //Setup
    EnchantBrokerDescribe_ProviderHasInvalidUtf8Describe_TestFixture():
            EnchantBrokerDescribeTestFixtureBase(List_Providers_ProviderConfigurationInvalidDescribe)
    {
        global_user_data = NULL;
    }
};

/**
 * enchant_broker_describe
 * @broker: A non-null #EnchantBroker
 * @fn: A non-null #EnchantBrokerDescribeFn
 * @user_data: Optional user-data
 *
 * Enumerates the Enchant providers and tells
 * you some rudimentary information about them.
 */


/*
 * Providers are discovered by probing first in the .enchant directory 
 * in the user's home directory. 
 * [on windows in the enchant directory in the user's Application Data
 *   directory]
 * 
 * The user's provider directory on windows can be overridden using the registry
 * setting HKEY_CURRENT_USER\Software\Enchant\Config\Data_Dir
 * 
 * Then from the module directory (that libenchant is in).
 *
 * The module directory can be overridden using the registry setting
 *    HKEY_CURRENT_USER\Software\Enchant\Config\Module_Dir
 * or HKEY_LOCAL_MACHINE\Software\Enchant\Config\Module_Dir
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantBrokerNoProvidersTestFixture, 
             EnchantBrokerDescribe_NoProviders)
{
    enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
    CHECK_EQUAL((unsigned int)0, _providerList.size());
}

TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_ProvidersInUserDirectoryTakePrecedenceOverProvidersInSystem)
{
    enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
    CHECK_EQUAL((unsigned int)2, _providerList.size());
    CHECK_EQUAL(2, std::count_if(_providerList.begin(), 
                                 _providerList.end(), 
                                 std::mem_fun_ref(&ProviderDescription::DataIsComplete)));
}

TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_NullUserData_PassedThroughToCallback)
{
    global_user_data = "hello world";

    enchant_broker_describe(_broker, 
                          EnchantBrokerDescribeAssignUserDataToStaticCallback,
                          NULL);
    CHECK_EQUAL((void*)NULL, global_user_data); 
}

TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_NonNullUserData_PassedThroughToCallback)
{
  char* data = "some data";
  global_user_data = NULL;
  enchant_broker_describe(_broker, 
                          EnchantBrokerDescribeAssignUserDataToStaticCallback,
                          data);
  CHECK(global_user_data);

  if(global_user_data){
      CHECK_EQUAL(data, (char*)global_user_data); 
  }
}

TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockProvider("something bad happened");

  enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);

  CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_NullBroker_DoNothing)
{
  enchant_broker_describe(NULL, EnchantBrokerDescribeCallback, &_providerList);
  CHECK_EQUAL((void*)NULL, global_user_data);
}

TEST_FIXTURE(EnchantBrokerDescribeTestFixture, 
             EnchantBrokerDescribe_NullDescribeFunction_DoNothing)
{
  enchant_broker_describe(_broker, NULL, &_providerList);
  CHECK_EQUAL((void*)NULL, global_user_data); 
}

TEST_FIXTURE(EnchantBrokerDescribe_ProviderLacksIdentify_TestFixture, 
             EnchantBrokerDescribe_ProviderLacksIdentify_NotLoaded)
{
  enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
  CHECK_EQUAL((unsigned int)1, _providerList.size());
  CHECK_EQUAL(1, std::count_if(_providerList.begin(), 
                               _providerList.end(), 
                               std::mem_fun_ref(&ProviderDescription::DataIsComplete)));
}

TEST_FIXTURE(EnchantBrokerDescribe_ProviderLacksDescribe_TestFixture, 
             EnchantBrokerDescribe_ProviderLacksDescribe_NotLoaded)
{
    enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
    CHECK_EQUAL((unsigned int)1, _providerList.size());
    CHECK_EQUAL(1, std::count_if(_providerList.begin(), 
                                 _providerList.end(), 
                                 std::mem_fun_ref(&ProviderDescription::DataIsComplete)));
}

TEST_FIXTURE(EnchantBrokerDescribe_ProviderHasInvalidUtf8Describe_TestFixture, 
             EnchantBrokerDescribe_ProviderHasInvalidUtf8Describe_NotLoaded)
{
    enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
    CHECK_EQUAL((unsigned int)1, _providerList.size());
    CHECK_EQUAL(1, std::count_if(_providerList.begin(), 
                                 _providerList.end(), 
                                 std::mem_fun_ref(&ProviderDescription::DataIsComplete)));
}

TEST_FIXTURE(EnchantBrokerDescribe_ProviderHasInvalidUtf8Identify_TestFixture, 
             EnchantBrokerDescribe_ProviderHasInvalidUtf8Identify_NotLoaded)
{
    enchant_broker_describe(_broker, EnchantBrokerDescribeCallback, &_providerList);
    CHECK_EQUAL((unsigned int)1, _providerList.size());
    CHECK_EQUAL(1, std::count_if(_providerList.begin(), 
                                 _providerList.end(), 
                                 std::mem_fun_ref(&ProviderDescription::DataIsComplete)));
}
