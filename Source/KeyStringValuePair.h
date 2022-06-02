#pragma once

namespace FFmpegInteropX
{
	
	using namespace Windows::Foundation::Collections;

	ref class KeyStringValuePair sealed : IKeyValuePair<winrt::hstring,winrt::hstring>
	{
		String ^key, ^value;

	public:

		property winrt::hstring Key
		{
			virtual winrt::hstring get()
			{
				return key;
			}
		}

		property winrt::hstring Value
		{
			virtual winrt::hstring get()
			{
				return value;
			}
		}

		KeyStringValuePair(winrt::hstring key, winrt::hstring value)
		{
			this->key = key;
			this->value = value;
		}
	};	
}