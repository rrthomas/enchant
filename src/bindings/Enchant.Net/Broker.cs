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

namespace Enchant
{
	public sealed class Broker : IDisposable
	{
		private readonly SafeBrokerHandle _handle;
		private IList<DictionaryInfo> _dictionaries;
		private bool _disposed = false;
		private IList<ProviderInfo> _providers;
	    private Dictionary<string, Dictionary> _dictionaryCache;
	    private Dictionary<string, Dictionary> _pwlDictionaryCache;
	    private bool _cacheDictionaries = true;

		public Broker()
		{
			_handle = Bindings.enchant_broker_init();
			VerifyNoErrors();
			if (_handle.IsInvalid)
			{
				throw new ApplicationException("Unable to initialize broker");
			}
            _dictionaryCache = new Dictionary<string, Dictionary>();
            _pwlDictionaryCache = new Dictionary<string, Dictionary>();
		}

		public IEnumerable<ProviderInfo> Providers
		{
			get
			{
				VerifyNotDisposed();
				if (_providers == null)
				{
					InitializeProviderList();
				}
				return _providers;
			}
		}

    private void InitializeProviderList()
    {
      _providers = new List<ProviderInfo>();
      Bindings.enchant_broker_describe(_handle,
                                       delegate(ProviderInfo provider)
                                       {
                                         _providers.Add(provider);
                                       });
      VerifyNoErrors();
    }

    public IEnumerable<DictionaryInfo> Dictionaries
		{
			get
			{
				VerifyNotDisposed();
				if (_dictionaries == null)
				{
					InitializeDictionaryList();
				}
				return _dictionaries;
			}
		}

	    public bool CacheDictionaries
	    {
	        get { return this._cacheDictionaries; }
	        set { this._cacheDictionaries = value; }
	    }

	    private void InitializeDictionaryList()
    {
      _dictionaries = new List<DictionaryInfo>();
      Bindings.enchant_broker_list_dicts(_handle,
                                         delegate(DictionaryInfo dictionary)
                                         {
                                           _dictionaries.Add(dictionary);
                                         });
      VerifyNoErrors();
    }

		#region IDisposable Members

    public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		#endregion

    private void Dispose(bool disposing)
    {
      if (!_disposed)
      {
        if (disposing)
        {
          // dispose-only, i.e. non-finalizable logic
          _handle.Dispose();
        }

        // shared (dispose and finalizable) cleanup logic
        _disposed = true;
      }
    }

    private void VerifyNotDisposed()
    {
      if (_disposed)
      {
        throw new ObjectDisposedException("Dictionary");
      }
    }

		public Dictionary RequestDictionary(string language_tag)
		{
			VerifyNotDisposed();
			if (language_tag == null)
			{
				throw new ArgumentNullException("language_tag");
			}
		    Dictionary dictionary;
		    if(CacheDictionaries)
            {
                if (_dictionaryCache.TryGetValue(language_tag, out dictionary))
                {
                    return dictionary;
                }
            }
			SafeDictionaryHandle handle = Bindings.enchant_broker_request_dict(_handle, language_tag);
			VerifyNoErrors();
			if (handle.IsInvalid)
			{
				throw new ApplicationException("There is no provider that supplies a dictionary for " + language_tag);
			}
		    dictionary = new Dictionary(handle);
            if(CacheDictionaries)
            {
                _dictionaryCache[language_tag] = dictionary;
            }
		    return dictionary;
		}

    public bool DictionaryExists(string language_tag)
    {
      VerifyNotDisposed();
      if (language_tag == null)
      {
        throw new ArgumentNullException("language_tag");
      }
      int result = Bindings.enchant_broker_dict_exists(_handle, language_tag);
      VerifyNoErrors();
      if (result != 0 && result != 1)
      {
        throw new NotImplementedException(
          "enchant_broker_dict_exists returned unexpected value that is currently unhandled.");
      }
      return result == 1;
    }

    public Dictionary RequestPwlDictionary(string pwlFile)
		{
			VerifyNotDisposed();
			if (pwlFile == null)
			{
				throw new ArgumentNullException("pwlFile");
			}
            Dictionary dictionary;
            if (CacheDictionaries)
            {
                if (_dictionaryCache.TryGetValue(pwlFile, out dictionary))
                {
                    return dictionary;
                }
            }

			SafeDictionaryHandle handle = Bindings.enchant_broker_request_pwl_dict(_handle, pwlFile);
			VerifyNoErrors();
			if (handle.IsInvalid)
			{
				throw new ApplicationException("Unable to create pwl file " + pwlFile);
			}
            dictionary = new Dictionary(handle);
            if (CacheDictionaries)
            {
                _dictionaryCache[pwlFile] = dictionary;
            }
            return dictionary;
		}

		public void SetOrdering(string language_tag, string ordering)
		{
			VerifyNotDisposed();
			Bindings.enchant_broker_set_ordering(_handle, language_tag, ordering);
			VerifyNoErrors();
		}

		private void VerifyNoErrors()
		{
			string message = Bindings.enchant_broker_get_error(_handle);
			if (!string.IsNullOrEmpty(message))
			{
				throw new ApplicationException(message);
			}
		}
	}
}