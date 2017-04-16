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
#include <fstream>
#include "EnchantBrokerTestFixture.h"

static bool mock1ProviderRequestDictionaryCalled;
static bool mock2ProviderRequestDictionaryCalled;

static EnchantDict * RequestDictionary1 (EnchantProvider *me, const char *tag)
{
	mock1ProviderRequestDictionaryCalled = true;
	return MockEnGbAndQaaProviderRequestDictionary(me, tag);
}
static EnchantDict * RequestDictionary2 (EnchantProvider *me, const char *tag)
{
	mock2ProviderRequestDictionaryCalled = true;
	return MockEnGbAndQaaProviderRequestDictionary(me, tag);
}
static const char *
MockProvider1Identify (EnchantProvider *)
{
	return "mock1";
}

static const char *
MockProvider2Identify (EnchantProvider *)
{
	return "mock2";
}
static void Request_Dictionary_ProviderConfiguration1 (EnchantProvider * me, const char *)
{
	 me->request_dict = RequestDictionary1;
	 me->dispose_dict = MockProviderDisposeDictionary;
	 me->identify = MockProvider1Identify;
}
static void Request_Dictionary_ProviderConfiguration2 (EnchantProvider * me, const char *)
{
	 me->request_dict = RequestDictionary2;
	 me->dispose_dict = MockProviderDisposeDictionary;
	 me->identify = MockProvider2Identify;
}


enum ProviderOrder {
	Mock1ThenMock2=1,
	Mock2ThenMock1=2,
	ErrorBothCalled=3,
	ErrorNeitherCalled=4
};

struct EnchantBrokerSetOrdering_TestFixture : EnchantBrokerTestFixture
{
	//Setup
	EnchantBrokerSetOrdering_TestFixture():
			EnchantBrokerTestFixture(Request_Dictionary_ProviderConfiguration1,Request_Dictionary_ProviderConfiguration2)

	{  
		mock1ProviderRequestDictionaryCalled = false;
		mock2ProviderRequestDictionaryCalled = false;
	}

	ProviderOrder GetProviderOrder(const std::string & languageTag) 
	{
		ProviderOrder result = ErrorBothCalled;

		EnchantDict* dict = enchant_broker_request_dict(_broker, languageTag.c_str());

		if(mock2ProviderRequestDictionaryCalled && !mock1ProviderRequestDictionaryCalled)
		{
			result = Mock2ThenMock1;
		}
		else if(mock1ProviderRequestDictionaryCalled && !mock2ProviderRequestDictionaryCalled)
		{
			result = Mock1ThenMock2;
		}
		else if(!mock2ProviderRequestDictionaryCalled && !mock1ProviderRequestDictionaryCalled)
		{
			result = ErrorNeitherCalled;
		}
		
		FreeDictionary(dict);

		mock1ProviderRequestDictionaryCalled = false;
		mock2ProviderRequestDictionaryCalled = false;

		return result;
	}
};

struct EnchantBrokerFileSetOrdering_TestFixture: EnchantBrokerSetOrdering_TestFixture
{
	EnchantBrokerFileSetOrdering_TestFixture()
	{
		if(_broker){ // this is now freed so that individual tests can set up the file state they
					 // need before calling         _broker = enchant_broker_init ();
			enchant_broker_free (_broker);
			_broker = NULL;
		}
	}

	static void WriteStringToOrderingFile(const std::string& path, const std::string& contents)
	{
		CreateDirectory(path);
		std::ofstream file(AddToPath(path, "enchant.ordering").c_str());
		file << contents;
	}

};


/**
 * enchant_broker_set_ordering
 * @broker: A non-null #EnchantBroker
 * @tag: A non-null language tag (en_US)
 * @ordering: A non-null ordering (aspell,hunspell,hspell)
 *
 * Declares a preference of dictionaries to use for the language
 * described/referred to by @tag. The ordering is a comma delimited
 * list of provider names. As a special exception, the "*" tag can
 * be used as a language tag to declare a default ordering for any
 * language that does not explictly declare an ordering.
 */


/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation
TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_AsteriskForLanguage_SetsDefaultOrdering)
{
	enchant_broker_set_ordering(_broker, "*", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));

	enchant_broker_set_ordering(_broker, "*", "mock1,mock2");
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_NoSpaces)
{
	enchant_broker_set_ordering(_broker, "qaa", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_WhitespaceAroundProvider)
{
	enchant_broker_set_ordering(_broker, "qaa", "\n\f\t\r mock2\n \f\t\r,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_WhitespaceAroundProviderAfterComma)
{
	enchant_broker_set_ordering(_broker, "qaa", "aspell,\n\f\t \rmock2\n\f \r\t,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

/* Vertical tab is not considered to be whitespace in glib!
	See bug# 59388 http://bugzilla.gnome.org/show_bug.cgi?id=59388
*/
TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_VerticalTabSurroundingProvider_NotRemoved)
{
  enchant_broker_set_ordering(_broker, "qaa", "\vmock2,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));

  enchant_broker_set_ordering(_broker, "qaa", "mock2\v,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));

  enchant_broker_set_ordering(_broker, "qaa", "aspell,\vmock2,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));

  enchant_broker_set_ordering(_broker, "qaa", "aspell,mock2\v,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));

}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_SpecificLanguage_SetsOrderingForSpecific)
{
	enchant_broker_set_ordering(_broker, "qaa", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));

	enchant_broker_set_ordering(_broker, "qaa", "mock1,mock2");
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_UnknownProvider_Ignored)
{
	enchant_broker_set_ordering(_broker, "qaa", "unknown,mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}


TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_WhitespaceSurroundingLanguageTag_Removed)
{
	enchant_broker_set_ordering(_broker, "\n\r qaa \t\f", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

/* Vertical tab is not considered to be whitespace in glib!
	See bug# 59388 http://bugzilla.gnome.org/show_bug.cgi?id=59388
*/
TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_VerticalTabPreceedingLanguageTag_NotRemoved)
{
  enchant_broker_set_ordering(_broker, "*", "mock1,mock2");
  enchant_broker_set_ordering(_broker, "\vqaa", "mock2,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));
}

/* Vertical tab is not considered to be whitespace in glib!
	See bug# 59388 http://bugzilla.gnome.org/show_bug.cgi?id=59388
*/
TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_VerticalTabFollowingLanguageTag_NotRemoved)
{
  enchant_broker_set_ordering(_broker, "*", "mock1,mock2");
  enchant_broker_set_ordering(_broker, "qaa\v", "mock2,mock1");
  CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_AtSignInLanguageTag_RemovesToTail)
{
	enchant_broker_set_ordering(_broker, "en_GB@euro", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerDictExists_PeriodInLanguageTag_RemovesToTail)
{
	enchant_broker_set_ordering(_broker, "en_GB.UTF-8", "unknown,mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_HyphensInLanguageTag_SubstitutedWithUnderscore)
{
	enchant_broker_set_ordering(_broker, "en-GB", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_DifferentCaseInLanguageTag_SubstitutedWithCorrectCase)
{
	enchant_broker_set_ordering(_broker, "EN-gb", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_DifferentCaseInLanguageTagNoRegion_SubstitutedWithCorrectCase)
{
	enchant_broker_set_ordering(_broker, "QAA", "mock2,mock1");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture, 
			 EnchantBrokerSetOrdering_HasPreviousError_ErrorCleared)
{
  SetErrorOnMockProvider("something bad happened");

  enchant_broker_set_ordering(_broker, "qaa", "mock2,mock1");

  CHECK_EQUAL((void*)NULL, (void*)enchant_broker_get_error(_broker));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_NullBroker_DoNothing)
{
	enchant_broker_set_ordering(_broker, "qaa", "mock2,mock1");
	enchant_broker_set_ordering(NULL, "qaa", "mock1,mock2");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_NullLanguageTag_DoNothing)
{
	enchant_broker_set_ordering(_broker, "*", "mock2,mock1");
	enchant_broker_set_ordering(_broker, NULL, "mock1,mock2");
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_EmptyAfterNormalization_DoNothing)
{
	enchant_broker_set_ordering(_broker, "  @  ", "mock1,mock2");
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_EmptyLanguageTag_DoNothing)
{
	enchant_broker_set_ordering(_broker, "", "aspell,hunspell");
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_NullOrdering_DoNothing)
{
	enchant_broker_set_ordering(_broker, "en_GB", NULL);
}

TEST_FIXTURE(EnchantBrokerSetOrdering_TestFixture,
			 EnchantBrokerSetOrdering_EmptyOrdering_DoNothing)
{
	enchant_broker_set_ordering(_broker, "en_GB", "");
}


/*
 * Ordering can also be set in enchant.ordering file:
 * Language_Tag : Provider1, Provider2, ProviderN
 *
 * The enchant.ordering file is discovered by first looking in the user's
 * config directory, then in the .enchant directory in the user's home directory.
 * 
 * The user's config directory is located at share/enchant 
 * in the module directory (that libenchant is in).
 */

/////////////////////////////////////////////////////////////////////////////
// Test Normal Operation

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
EnchantBrokerFileOrderingMock1ThenMock2_DefaultConfigDirectory)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock1,mock2");
	InitializeBroker();

	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
EnchantBrokerFileOrderingMock2ThenMock1_DefaultConfigDirectory)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock2,mock1");
	InitializeBroker();

	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

// FIXME: Resurrect these once we have a way to reconfigure user config directory
#if 0
TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
EnchantBrokerFileOrderingMock1ThenMock2_UserOverriddenConfigDirectory)
{
	WriteStringToOrderingFile(_tempPath, "en_GB:mock1,mock2");
	SetUserRegistryConfigDir(_tempPath);
	InitializeBroker();

	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrderingMock2ThenMock1_UserOverriddenConfigDirectory)
{
	WriteStringToOrderingFile(_tempPath, "en_GB:mock2,mock1");
	SetUserRegistryConfigDir(_tempPath);
	InitializeBroker();

	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrderingMock1ThenMock2_MachineOverriddenConfigDirectory)
{
	WriteStringToOrderingFile(_tempPath, "en_GB:mock1,mock2");
	SetMachineRegistryConfigDir(_tempPath);
	InitializeBroker();
	
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrderingMock2ThenMock1_MachineOverriddenConfigDirectory)
{
	WriteStringToOrderingFile(_tempPath, "en_GB:mock2,mock1");
	SetMachineRegistryConfigDir(_tempPath);
	InitializeBroker();
	
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

// if config registry "" global from share/enchant
TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_OverriddenConfigDirectoryIsBlank_UsesSettingsFromDefaultConfigDirectory_Mock1Then2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock1,mock2");
	SetMachineRegistryConfigDir("");
	InitializeBroker();
	
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_OverriddenConfigDirectoryIsBlank_UsesSettingsFromDefaultConfigDirectory_Mock2Then1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock2,mock1");
	SetMachineRegistryConfigDir("");
	InitializeBroker();
	
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}
#endif

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_ExtraSpacesAndTabs_Mock1Then2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"\t en_GB\f \t:\r\t mock1\t , \tmock2\t ");
	InitializeBroker();
	
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_ExtraSpaces_Mock2Then1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir()," \ten_GB\t \f:\r \tmock2 \t,\t mock1 \t");
	InitializeBroker();
	
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_ExtraNewLines_Mock1Then2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"\nen_GB:mock1,mock2\n\nqaa:mock2,mock1\n*:mock2\n");
	InitializeBroker();
	
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_ExtraNewLines_Mock2Then1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"\nen_GB:mock2,mock1\n\nqaa:mock2,mock1\n*:mock2\n");
	InitializeBroker();
	
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_HomeAndGlobal_HomeMergedWithGlobal_HomeTakesPrecedence_Mock1Then2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock2,mock1\nqaa:mock1,mock2");
	WriteStringToOrderingFile(GetTempUserEnchantDir(), "en_GB:mock1,mock2");

	InitializeBroker();
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("qaa"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_HomeAndGlobal_HomeMergedWithGlobal_HomeTakesPrecedence_Mock2Then1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock1,mock2\nqaa:mock2,mock1");
	WriteStringToOrderingFile(GetTempUserEnchantDir(), "en_GB:mock2,mock1");

	InitializeBroker();
	
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("qaa"));
}

/////////////////////////////////////////////////////////////////////////////
// Test Error Conditions
TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_EmptyFile)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"");

	InitializeBroker();
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_NoLanguageTag)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),":mock1,mock2");

	InitializeBroker();
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_NoColon)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB  mock1,mock2");

	InitializeBroker();
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_NoProviders_DoesNotOverridePreviousOrdering_Mock1Then2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:");
	WriteStringToOrderingFile(GetTempUserEnchantDir(), "en_GB:mock1,mock2");

	InitializeBroker();

	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
 EnchantBrokerFileOrdering_NoProviders_DoesNotOverridePreviousOrdering_Mock2Then1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:");
	WriteStringToOrderingFile(GetTempUserEnchantDir(), "en_GB:mock2,mock1");

	InitializeBroker();
	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
EnchantBrokerFileOrdering_ListedTwice_LastTakesPrecedence_Mock1ThenMock2)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock2,mock1\nen_GB:mock1,mock2");
	InitializeBroker();

	CHECK_EQUAL(Mock1ThenMock2, GetProviderOrder("en_GB"));
}

TEST_FIXTURE(EnchantBrokerFileSetOrdering_TestFixture,
EnchantBrokerFileOrdering_ListedTwice_LastTakesPrecedence_Mock2ThenMock1)
{
	WriteStringToOrderingFile(GetEnchantConfigDir(),"en_GB:mock1,mock2\nen_GB:mock2,mock1");
	InitializeBroker();

	CHECK_EQUAL(Mock2ThenMock1, GetProviderOrder("en_GB"));
}
