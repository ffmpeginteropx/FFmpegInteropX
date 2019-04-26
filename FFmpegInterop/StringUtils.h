#pragma once

#include<string>
using namespace Platform;
namespace FFmpegInterop
{

	ref class StringUtils
	{
	internal:
		static std::string* PlatformStringToChar(String^ value)
		{
			std::wstring strW(value->Begin());
			std::string* strA = new std::string(strW.begin(), strW.end());

			return strA;
		}
	};
}

