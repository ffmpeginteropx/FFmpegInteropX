#pragma once
using namespace Platform;
using namespace Windows::Foundation::Collections;

namespace FFmpegInterop {
	public ref class KeyStringValuePair sealed 
	{
		String ^key, ^value;

	public:

		property String^ Key
		{
			String^ get()
			{
				return key;
			}
		}

		property String^ Value
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