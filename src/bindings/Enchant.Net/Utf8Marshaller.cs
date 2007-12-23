/* Copyright (c) 2007 Christopher Wilks and Eric Scott Albright
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
using System.Text;

namespace Enchant
{
	internal sealed class Utf8Marshaller : IDisposable
	{
		private readonly int _byteCount;
		private readonly IntPtr _unmanagedPrt;
		private bool _isDisposed;

		public Utf8Marshaller(string value)
		{
			_byteCount = Encoding.UTF8.GetByteCount(value);
			byte[] bytes = new byte[_byteCount + 1];
			int bytesWritten = Encoding.UTF8.GetBytes(value, 0, value.Length, bytes, 0);
			bytes[bytesWritten] = 0; // null terminate it
			++bytesWritten;
			_unmanagedPrt = Marshal.AllocHGlobal(bytesWritten);
			for (int i = 0; i < bytes.Length; i++)
			{
				Marshal.WriteByte(MarshalledValue, i, bytes[i]);
			}
		}

		public IntPtr MarshalledValue
		{
			get { return _unmanagedPrt; }
		}

		public int MarshalledSize
		{
			get { return _byteCount; }
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
			if (_isDisposed)
			{
				return;
			}
			_isDisposed = true;
			if (disposing)
			{
				//dump managed resources
			}

			Marshal.FreeHGlobal(MarshalledValue);
		}

		public static List<string> MarshalFromUtf8Array(IntPtr listAddress, int count)
		{
			List<string> strings = new List<string>();

			for (int i = 0; i < count; i++)
			{
				IntPtr stringAddress = Marshal.ReadIntPtr(listAddress, i*IntPtr.Size);
				string s = MarshalFromUtf8(stringAddress);
				strings.Add(s);
			}
			return strings;
		}

		public static string MarshalFromUtf8(IntPtr intPtr)
		{
			if (intPtr.ToInt64() == 0)
			{
				return null;
			}
			List<byte> bytes = new List<byte>();
			for (int i = 0; true; i++)
			{
				byte b = Marshal.ReadByte(intPtr, i);
				if (b == 0)
				{
					break;
				}
				bytes.Add(b);
			}
			return Encoding.UTF8.GetString(bytes.ToArray());
		}

		~Utf8Marshaller()
		{
			Dispose(false);
		}
	}
}