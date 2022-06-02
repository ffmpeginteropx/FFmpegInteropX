#pragma once
#include "pch.h"
#include "KeyStringValuePair.g.h"

namespace winrt::FFmpegInteropX::implementation
{
	using namespace winrt::Windows::Foundation::Collections;

	struct KeyStringValuePair : KeyStringValuePairT<KeyStringValuePair>, IKeyValuePair<winrt::hstring, winrt::hstring>
	{
		winrt::hstring key, value;

	public:

		winrt::hstring Key()
		{
			return key;
		}

		winrt::hstring Value()
		{
			return value;
		}

		KeyStringValuePair(winrt::hstring key, winrt::hstring value)
		{
			this->key = key;
			this->value = value;
		}
	};
}

namespace winrt::FFmpegInteropX::factory_implementation
{
	struct KeyStringValuePair : KeyStringValuePairT<KeyStringValuePair, implementation::KeyStringValuePair>
	{
	};
}