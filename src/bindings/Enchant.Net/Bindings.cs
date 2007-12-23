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
using System.Runtime.InteropServices;

namespace Enchant
{
	internal static class Bindings
	{
		#region Delegates

		public delegate void EnchantBrokerDescribeDelegate(ProviderInfo provider_info);

		public delegate void EnchantDictDescribeDelegate(DictionaryInfo dictionary_info);

		#endregion

		private const string ENCHANT_LIBRARY = "libenchant.dll";

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		public static extern SafeBrokerHandle enchant_broker_init();

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		public static extern void enchant_broker_free(IntPtr broker);

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr enchant_broker_request_dict(SafeBrokerHandle broker, IntPtr tag);

		public static SafeDictionaryHandle enchant_broker_request_dict(SafeBrokerHandle broker,
																																	 string tag)
		{
			using (Utf8Marshaller utf8Tag = new Utf8Marshaller(tag))
			{
				IntPtr handle = enchant_broker_request_dict(broker, utf8Tag.MarshalledValue);
				return new SafeDictionaryHandle(broker, handle);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr enchant_broker_request_pwl_dict(
			SafeBrokerHandle broker, IntPtr pwl);

		public static SafeDictionaryHandle enchant_broker_request_pwl_dict(SafeBrokerHandle broker,
																																			 string pwl)
		{
			using (Utf8Marshaller utf8Pwl = new Utf8Marshaller(pwl))
			{
				IntPtr handle = enchant_broker_request_pwl_dict(broker, utf8Pwl.MarshalledValue);
				return new SafeDictionaryHandle(broker, handle);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		public static extern void enchant_broker_free_dict(SafeBrokerHandle broker, IntPtr dict);

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern int enchant_broker_dict_exists(SafeBrokerHandle broker, IntPtr tag);

		public static int enchant_broker_dict_exists(SafeBrokerHandle broker, string tag)
		{
			using (Utf8Marshaller utf8Tag = new Utf8Marshaller(tag))
			{
				return enchant_broker_dict_exists(broker, utf8Tag.MarshalledValue);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_broker_set_ordering(SafeBrokerHandle broker,
																													 IntPtr tag,
																													 IntPtr ordering);

		public static void enchant_broker_set_ordering(SafeBrokerHandle broker,
																									 string tag,
																									 string ordering)
		{
			using (
				Utf8Marshaller utf8Tag = new Utf8Marshaller(tag),
											 utf8Ordering = new Utf8Marshaller(ordering))
			{
				enchant_broker_set_ordering(broker,
																		utf8Tag.MarshalledValue,
																		utf8Ordering.MarshalledValue);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		public static extern string enchant_broker_get_error(SafeBrokerHandle broker);

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_broker_describe(SafeBrokerHandle broker,
																											 [MarshalAs(UnmanagedType.FunctionPtr)] EnchantBrokerDescribeFn
																												 enchantBrokerDescribeFn,
																											 IntPtr user_data);

		public static void enchant_broker_describe(SafeBrokerHandle broker,
																							 EnchantBrokerDescribeDelegate describe)
		{
			enchant_broker_describe(broker,
															delegate(IntPtr provider_name,
																			 IntPtr provider_desc,
																			 IntPtr provider_dll_file,
																			 IntPtr user_data)
																{
																	ProviderInfo provider =
																		new ProviderInfo(
																			Utf8Marshaller.MarshalFromUtf8(provider_name),
																			Utf8Marshaller.MarshalFromUtf8(provider_desc),
																			Utf8Marshaller.MarshalFromUtf8(
																				provider_dll_file));
																	describe(provider);
																},
															IntPtr.Zero);
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_broker_list_dicts(SafeBrokerHandle broker,
																												 [MarshalAs(UnmanagedType.FunctionPtr)] EnchantDictDescribeFn
																													 enchantDictDescribeFn,
																												 IntPtr user_data);

		public static void enchant_broker_list_dicts(SafeBrokerHandle broker,
																								 EnchantDictDescribeDelegate describe)
		{
			enchant_broker_list_dicts(broker,
																delegate(IntPtr lang_tag,
																				 IntPtr provider_name,
																				 IntPtr provider_desc,
																				 IntPtr provider_dll_file,
																				 IntPtr user_data)
																	{
																		ProviderInfo provider =
																			new ProviderInfo(
																				Utf8Marshaller.MarshalFromUtf8(provider_name),
																				Utf8Marshaller.MarshalFromUtf8(provider_desc),
																				Utf8Marshaller.MarshalFromUtf8(provider_dll_file));
																		DictionaryInfo dictionary =
																			new DictionaryInfo(
																				Utf8Marshaller.MarshalFromUtf8(lang_tag),
																				provider);
																		describe(dictionary);
																	},
																IntPtr.Zero);
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_describe(SafeDictionaryHandle dict,
																										 [MarshalAs(UnmanagedType.FunctionPtr)] EnchantDictDescribeFn
																											 enchantDictDescribeFn,
																										 IntPtr user_data);

		public static void enchant_dict_describe(SafeDictionaryHandle dict,
																						 EnchantDictDescribeDelegate describe)
		{
			enchant_dict_describe(dict,
														delegate(IntPtr lang_tag,
																		 IntPtr provider_name,
																		 IntPtr provider_desc,
																		 IntPtr provider_dll_file,
																		 IntPtr user_data)
															{
																ProviderInfo provider =
																	new ProviderInfo(
																		Utf8Marshaller.MarshalFromUtf8(provider_name),
																		Utf8Marshaller.MarshalFromUtf8(provider_desc),
																		Utf8Marshaller.MarshalFromUtf8(provider_dll_file));
																DictionaryInfo dictionary =
																	new DictionaryInfo(
																		Utf8Marshaller.MarshalFromUtf8(lang_tag),
																		provider);
																describe(dictionary);
															},
														IntPtr.Zero);
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern int enchant_dict_check(SafeDictionaryHandle dict, IntPtr word, int len);

		public static int enchant_dict_check(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				return enchant_dict_check(dict, utf8Word.MarshalledValue, utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr enchant_dict_suggest(SafeDictionaryHandle dict,
																											IntPtr word,
																											int len,
																											[MarshalAs(UnmanagedType.U4)] out int
																												out_n_suggs);

		public static List<string> enchant_dict_suggest(SafeDictionaryHandle dict, string word)
		{
			int suggsCount;
			IntPtr suggs;
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				suggs =
					enchant_dict_suggest(dict,
															 utf8Word.MarshalledValue,
															 utf8Word.MarshalledSize,
															 out suggsCount);
			}
			List<string> result = Utf8Marshaller.MarshalFromUtf8Array(suggs, suggsCount);
			enchant_dict_free_string_list(dict, suggs);
			return result;
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_free_string_list(SafeDictionaryHandle dict,
																														 IntPtr string_list);

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_add(SafeDictionaryHandle dict, IntPtr word, int len);

		public static void enchant_dict_add(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				enchant_dict_add(dict, utf8Word.MarshalledValue, utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_add_to_session(SafeDictionaryHandle dict,
																													 IntPtr word,
																													 int len);

		public static void enchant_dict_add_to_session(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				enchant_dict_add_to_session(dict, utf8Word.MarshalledValue, utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_remove(SafeDictionaryHandle dict,
																									 IntPtr word,
																									 int len);

		public static void enchant_dict_remove(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				enchant_dict_remove(dict, utf8Word.MarshalledValue, utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_remove_from_session(SafeDictionaryHandle dict,
																																IntPtr word,
																																int len);

		public static void enchant_dict_remove_from_session(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				enchant_dict_remove_from_session(dict,
																				 utf8Word.MarshalledValue,
																				 utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern int enchant_dict_is_added(SafeDictionaryHandle dict,
																										IntPtr word,
																										int len);

		public static int enchant_dict_is_added(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				return
					enchant_dict_is_added(dict,
																utf8Word.MarshalledValue,
																utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern int enchant_dict_is_removed(SafeDictionaryHandle dict,
																											IntPtr word,
																											int len);

		public static int enchant_dict_is_removed(SafeDictionaryHandle dict, string word)
		{
			using (Utf8Marshaller utf8Word = new Utf8Marshaller(word))
			{
				return
					enchant_dict_is_removed(dict,
																	utf8Word.MarshalledValue,
																	utf8Word.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
		private static extern void enchant_dict_store_replacement(SafeDictionaryHandle dict,
																															IntPtr mis,
																															int mis_len,
																															IntPtr cor,
																															int cor_len);

		public static void enchant_dict_store_replacement(SafeDictionaryHandle dict,
																											string mis,
																											string cor)
		{
			using (
				Utf8Marshaller utf8Mis = new Utf8Marshaller(mis),
											 utf8Cor = new Utf8Marshaller(cor))
			{
				enchant_dict_store_replacement(dict,
																			 utf8Mis.MarshalledValue,
																			 utf8Mis.MarshalledSize,
																			 utf8Cor.MarshalledValue,
																			 utf8Cor.MarshalledSize);
			}
		}

		[DllImport(ENCHANT_LIBRARY, CallingConvention = CallingConvention.Cdecl,
			EntryPoint = "enchant_dict_get_error")]
		private static extern IntPtr enchant_dict_get_error_(SafeDictionaryHandle dict);

		public static string enchant_dict_get_error(SafeDictionaryHandle dict)
		{
			IntPtr message = enchant_dict_get_error_(dict);
			return Utf8Marshaller.MarshalFromUtf8(message);
		}

		#region Nested type: EnchantBrokerDescribeFn

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void EnchantBrokerDescribeFn(
			IntPtr provider_name,
			IntPtr provider_desc,
			IntPtr provider_dll_file,
			IntPtr user_data);

		#endregion

		#region Nested type: EnchantDictDescribeFn

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void EnchantDictDescribeFn(
			IntPtr lang_tag,
			IntPtr provider_name,
			IntPtr provider_desc,
			IntPtr provider_file,
			IntPtr user_data);

		#endregion
	}
}