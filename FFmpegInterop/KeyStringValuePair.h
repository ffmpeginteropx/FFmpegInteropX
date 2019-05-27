#pragma once
using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace FFmpegInterop {
	public ref class KeyStringValuePair sealed : IKeyValuePair<String^, String^>
	{
		String ^key, ^value;

	public:

		virtual property String^ Key
		{
			String^ get()
			{
				return key;
			}
		}

		virtual property String^ Value
		{
			String^ get()
			{
				return value;
			}
		}

		KeyStringValuePair(String^ key, String^ value)
		{
			this->key = key;
			this->value = value;
		}
	};	
}