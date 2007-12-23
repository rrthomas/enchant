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

namespace Enchant
{
	public class DictionaryInfo
	{
		private readonly string _language_tag;
		private readonly ProviderInfo _provider_info;

		internal DictionaryInfo(string language_tag, ProviderInfo provider_info)
		{
			if (language_tag == null)
			{
				throw new ArgumentNullException("language_tag");
			}
			if (provider_info == null)
			{
				throw new ArgumentNullException("provider_info");
			}
			_language_tag = language_tag;
			_provider_info = provider_info;
		}

		public string Language
		{
			get { return _language_tag; }
		}

		public ProviderInfo Provider
		{
			get { return _provider_info; }
		}
	}
}