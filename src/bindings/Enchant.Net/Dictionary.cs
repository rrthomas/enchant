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
	public sealed class Dictionary : IDisposable
	{
		private readonly SafeDictionaryHandle _handle;
		private bool _disposed = false;
		private DictionaryInfo _info;

		internal Dictionary(SafeDictionaryHandle handle)
		{
			if (handle == null)
			{
				throw new ArgumentNullException("handle");
			}
			if (handle.IsInvalid)
			{
				throw new ArgumentException("handle is invalid");
			}
			_handle = handle;
		}

		public DictionaryInfo Information
		{
			get
			{
				VerifyNotDisposed();
				if (_info == null)
				{
					InitializeDictionaryInformation();
				}
				return _info;
			}
		}

    private void InitializeDictionaryInformation()
    {
      Bindings.enchant_dict_describe(_handle,
                                     delegate(DictionaryInfo dictionary)
                                     {
                                       _info = dictionary;
                                     });
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
        if (disposing)
        {
            OnDisposed(); // call it here so will throw if someone uses
            // it because _disposed has been set to true;
        }
      }
    }

    private void VerifyNotDisposed()
    {
      if (_disposed)
      {
        throw new ObjectDisposedException("Dictionary");
      }
    }

    public bool Check(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			// result is 0 if correctly spelled, positive if not, negative if error
			int result = Bindings.enchant_dict_check(_handle, word);
			if (result < 0)
			{
				string message = Bindings.enchant_dict_get_error(_handle);
				throw new ApplicationException(message);
			}
			return result == 0;
		}

		public ICollection<string> Suggest(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			return Bindings.enchant_dict_suggest(_handle, word);
		}

		public void Add(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			Bindings.enchant_dict_add(_handle, word);
		}

		public void AddToSession(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			Bindings.enchant_dict_add_to_session(_handle, word);
		}

		public bool IsAdded(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			return Bindings.enchant_dict_is_added(_handle, word) == 1;
		}

		public void Remove(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			Bindings.enchant_dict_remove(_handle, word);
		}

		public void RemoveFromSession(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			Bindings.enchant_dict_remove_from_session(_handle, word);
		}

		public bool IsRemoved(string word)
		{
			VerifyNotDisposed();
			if (word == null)
			{
				throw new ArgumentNullException("word");
			}
			return Bindings.enchant_dict_is_removed(_handle, word) == 1;
		}

		public void StoreReplacement(string misspelling, string correction)
		{
			VerifyNotDisposed();
			if (misspelling == null)
			{
				throw new ArgumentNullException("misspelling");
			}
			if (correction == null)
			{
				throw new ArgumentNullException("correction");
			}
			Bindings.enchant_dict_store_replacement(_handle, misspelling, correction);
		}

        /// <summary>
        /// Occurs when a dictionary is disposed by a call to the Dispose method
        /// </summary>
        public event EventHandler Disposed = delegate { };

        private void OnDisposed()
        {
            Disposed.Invoke(this, new EventArgs());
        }
	}
}