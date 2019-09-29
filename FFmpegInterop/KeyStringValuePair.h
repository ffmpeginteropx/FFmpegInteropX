#pragma once
using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace FFmpegInterop {
	ref class KeyStringValuePair sealed : IKeyValuePair<String^,String^>
	{
		String ^key, ^value;

	public:

		property String^ Key
		{
			virtual String^ get()
			{
				return key;
			}
		}

		property String^ Value
		{
			virtual String^ get()
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