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
	    private readonly Dictionary<string, WeakReference> _dictionaryCache;
	    private readonly Dictionary<string, WeakReference> _pwlDictionaryCache;
	    private bool _cacheDictionaries = true;

		public Broker()
		{
			_handle = Bindings.enchant_broker_init();
			VerifyNoErrors();
			if (_handle.IsInvalid)
			{
				throw new ApplicationException("Unable to initialize broker");
			}
            _dictionaryCache = new Dictionary<string, WeakReference>();
            _pwlDictionaryCache = new Dictionary<string, WeakReference>();
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
                DisposeAllDictionariesFromCache(_dictionaryCache);
                DisposeAllDictionariesFromCache(_pwlDictionaryCache);
            }

            // shared (dispose and finalizable) cleanup logic
            _disposed = true;
          }
        }

	    private static void DisposeAllDictionariesFromCache(ICollection<KeyValuePair<string, WeakReference>> cache) {
	        List<Dictionary> dictionariesToDispose = new List<Dictionary>();
            foreach (KeyValuePair<string, WeakReference> pair in cache)
	        {
	            if(pair.Value.IsAlive)
	            {
	                dictionariesToDispose.Add((Dictionary) pair.Value.Target);
	            }
	        }
            cache.Clear();
	        foreach (Dictionary dictionary in dictionariesToDispose)
	        {
                dictionary.Dispose();
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
            Dictionary dictionary = GetDictionaryFromCache(this._dictionaryCache, language_tag);
            if(dictionary != null)
            {
                return dictionary;
            }
	        return CreateDictionary(language_tag);
	    }

	    private Dictionary CreateDictionary(string language_tag) {
	        SafeDictionaryHandle handle = Bindings.enchant_broker_request_dict(this._handle, language_tag);
	        VerifyNoErrors();
	        if (handle.IsInvalid)
	        {
	            throw new ApplicationException("There is no provider that supplies a dictionary for " + language_tag);
	        }

	        return CreateAndRegisterDictionary(handle, this._dictionaryCache, language_tag);
	    }

	    private Dictionary GetDictionaryFromCache(IDictionary<string, WeakReference> cache, string language_tag) {
	        if(CacheDictionaries)
	        {
	            WeakReference dictionaryReference;
	            if (cache.TryGetValue(language_tag, out dictionaryReference))
	            {
                    if (dictionaryReference.IsAlive)
                    {
                        return (Dictionary) dictionaryReference.Target;
                    }
	            }
	        }
	        return null;
	    }

	    private Dictionary CreateAndRegisterDictionary(SafeDictionaryHandle handle, IDictionary<string, WeakReference> cache, string language_tag) {
	        Dictionary dictionary;
	        dictionary = new Dictionary(handle);
	        dictionary.Disposed += OnDictionaryDisposed;
	        // always store the dictionaries we have created
	        // so that we can dispose of them cleanly and give a 
	        // better error message (ObjectDisposed) instead of a crash
	        // if someone tries to use a dictionary after the broker
	        // that created it has been disposed.
	        cache[language_tag] = new WeakReference(dictionary);
	        return dictionary;
	    }

	    private void OnDictionaryDisposed(object sender, EventArgs e)
	    {
	        Dictionary dictionary = (Dictionary) sender;

            // try to remove from _dictionaryCache
            if (!RemoveDictionaryFromCache(this._dictionaryCache, dictionary))
            {
                // try to remove from _pwlDictionaryCache
                RemoveDictionaryFromCache(this._pwlDictionaryCache, dictionary);
            }
	    }

	    private static bool RemoveDictionaryFromCache(IDictionary<string, WeakReference> cache, Dictionary dictionary) {
	        foreach (KeyValuePair<string, WeakReference> pair in cache)
	        {
                if (pair.Value.IsAlive 
                    && pair.Value.Target == dictionary)
                {
                    cache.Remove(pair.Key);
                    return true;
                }
	        }
	        return false;
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

            Dictionary dictionary = GetDictionaryFromCache(this._pwlDictionaryCache, pwlFile);
            if (dictionary != null)
            {
                return dictionary;
            }

			return CreatePwlDictionary(pwlFile);
		}

	    private Dictionary CreatePwlDictionary(string pwlFile) {
	        SafeDictionaryHandle handle = Bindings.enchant_broker_request_pwl_dict(this._handle, pwlFile);
	        VerifyNoErrors();
	        if (handle.IsInvalid)
	        {
	            throw new ApplicationException("Unable to create pwl file " + pwlFile);
	        }
	        return CreateAndRegisterDictionary(handle, this._pwlDictionaryCache, pwlFile);
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