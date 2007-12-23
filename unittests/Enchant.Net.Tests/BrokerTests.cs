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

using System;
using System.Collections.Generic;
using System.IO;
using Microsoft.Win32;
using NUnit.Framework;

namespace Enchant.Tests
{
	[TestFixture]
	public class BrokerTests
	{
		#region Setup/Teardown

		[SetUp]
		public void Setup()
		{
			oldRegistryValue = (string)
												 Registry.GetValue(@"HKEY_CURRENT_USER\Software\Enchant\Config", "Data_Dir", null);
			tempdir = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());

			Registry.SetValue(@"HKEY_CURRENT_USER\Software\Enchant\Config", "Data_Dir", tempdir, RegistryValueKind.String);
		}

		[TearDown]
		public void Teardown()
		{
			if (oldRegistryValue == null)
			{
				Registry.CurrentUser.OpenSubKey("Software").OpenSubKey("Enchant").OpenSubKey("Config", true).DeleteValue(
					"Data_Dir");
			}
			else
			{
				Registry.SetValue(@"HKEY_CURRENT_USER\Software\Enchant\Config",
													"Data_Dir",
													oldRegistryValue,
													RegistryValueKind.String);
			}
			while (Directory.Exists(tempdir))
			{
				Directory.Delete(tempdir, true);
			}
		}

		#endregion

		private string tempdir;
		private string oldRegistryValue;

		[TestFixtureSetUp]
		public void FixtureSetup()
		{
			string providerDir = Path.Combine(Path.Combine(
																					Directory.GetCurrentDirectory(), "lib"), "enchant");
			if (!Directory.Exists(providerDir))
			{
				Directory.CreateDirectory(providerDir);
			}
			File.Copy("libenchant_ispell.dll",
								Path.Combine(providerDir, "libenchant_ispell.dll"), true);
			File.Copy("libenchant_myspell.dll",
								Path.Combine(providerDir, "libenchant_myspell.dll"), true);

			string dictionarySourceDir =
				Path.Combine(Path.Combine(Path.Combine(Path.Combine(Path.Combine(Path.Combine(
																																					 Directory.GetCurrentDirectory(), ".."), ".."),
																														"lib"), "share"),
																	"enchant"), "myspell");

			string dictionaryDestDir = Path.Combine(Path.Combine(Path.Combine(
																														 Directory.GetCurrentDirectory(), "share"), "enchant"),
																							"myspell");

			if (!Directory.Exists(dictionaryDestDir))
			{
				Directory.CreateDirectory(dictionaryDestDir);
			}

			File.Copy(Path.Combine(dictionarySourceDir, "en_US.aff"),
								Path.Combine(dictionaryDestDir, "en_US.aff"), true);

			File.Copy(Path.Combine(dictionarySourceDir, "en_US.dic"),
								Path.Combine(dictionaryDestDir, "en_US.dic"), true);
		}

		[TestFixtureTearDown]
		public void FixtureTearDown()
		{
			Directory.Delete(Path.Combine(Directory.GetCurrentDirectory(), "lib"), true);
			Directory.Delete(Path.Combine(Directory.GetCurrentDirectory(), "share"), true);
		}

		[Test]
		public void Construct()
		{
			Assert.IsNotNull(new Broker());
		}

		[Test]
		public void Dictionaries()
		{
			using (Broker broker = new Broker())
			{
				IEnumerable<DictionaryInfo> dictionaries = broker.Dictionaries;
				Assert.IsNotNull(dictionaries);
				int count = 0;
				foreach (DictionaryInfo info in dictionaries)
				{
					Console.WriteLine("Language:{0}\tName:{1}\tDescription:{2}\tFile:{3}",
														info.Language,
														info.Provider.Name,
														info.Provider.Description,
														info.Provider.File);
					Assert.IsNotEmpty(info.Language);
					Assert.IsNotEmpty(info.Provider.Name);
					Assert.IsNotEmpty(info.Provider.Description);
					Assert.IsNotEmpty(info.Provider.File);
					++count;
				}
				Assert.AreEqual(1, count);
			}
		}

		[Test]
		public void DictionaryExists()
		{
			using (Broker broker = new Broker())
			{
				Assert.IsFalse(broker.DictionaryExists("qaa"));
				Assert.IsTrue(broker.DictionaryExists("en_US"));
			}
		}

		[Test]
		public void Providers()
		{
			using (Broker broker = new Broker())
			{
				IEnumerable<ProviderInfo> providers = broker.Providers;
				Assert.IsNotNull(providers);
				int count = 0;
				foreach (ProviderInfo info in providers)
				{
					Console.WriteLine("Name:{0}\tDescription:{1}\tFile:{2}",
														info.Name,
														info.Description,
														info.File);
					Assert.IsNotEmpty(info.Name);
					Assert.IsNotEmpty(info.Description);
					Assert.IsNotEmpty(info.File);
					++count;
				}
				Assert.AreEqual(2, count);
			}
		}

		[Test]
		public void RequestDictionary()
		{
			using (Broker broker = new Broker())
			{
				Dictionary dictionary = broker.RequestDictionary("en_US");
				Assert.IsNotNull(dictionary);
			}
		}

		[Test]
		[ExpectedException(typeof (ApplicationException))]
		public void RequestDictionary_DictionaryDoesNotExist_Throws()
		{
			using (Broker broker = new Broker())
			{
				broker.RequestDictionary("qaa");
			}
		}

		[Test]
		public void RequestPwlDictionary()
		{
			string filename = Path.GetTempFileName();
			using (Broker broker = new Broker())
			{
				using (Dictionary dictionary = broker.RequestPwlDictionary(filename))
				{
					Assert.IsNotNull(dictionary);
					File.Delete(filename);
				}
			}
		}

		[Test]
		public void SetOrdering()
		{
			using (Broker broker = new Broker())
			{
				broker.SetOrdering("en_US", "aspell, myspell, ispell");
			}
		}
	}
}