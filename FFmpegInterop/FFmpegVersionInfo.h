#pragma once

#include <winerror.h>

extern "C"
{
#include <libavformat/avformat.h>
}

using namespace Platform;
using namespace Windows::Foundation;

String^ ConvertString(const char* charString);

void CheckFFmpegVersion(String^ current, String^ min)
{
	int v1, v2, v3, min1, min2, min3;

	auto countV = swscanf_s(current->Data(),
		L"%i.%i.%i",
		&v1, &v2, &v3);

	auto countMin = swscanf_s(min->Data(),
		L"%i.%i.%i",
		&min1, &min2, &min3);

	if (countV == 2)
	{
		v3 = 0;
	}

	if (countMin == 2)
	{
		min3 = 0;
	}

	if (countV < 2 || countMin < 2)
	{
		throw ref new COMException(E_UNEXPECTED);
	}
	else if (min1 > v1 || (min1 == v1 && min2 > v2) || (min1 == v1 && min2 == v2 && min3 > v3))
	{
		throw ref new COMException(E_FAIL);
	}
}

namespace FFmpegInterop
{

	public ref class FFmpegVersionInfo sealed
	{
	public:
		static property String^ MinimumVersion { String^ get() { return "4.0"; } };
		static property String^ RecommendedVersion { String^ get() { return "4.1"; } };
		static property String^ CurrentVersion 
		{ 
			String^ get()
			{
				auto version = av_version_info();
				if (version && version[0] == 'n')
				{
					version = version++;
				}
				return ConvertString(version);
			}
		};

		static void CheckMinimumVersion()
		{
			CheckFFmpegVersion(CurrentVersion, MinimumVersion);
		}

		static void CheckRecommendedVersion()
		{
			CheckFFmpegVersion(CurrentVersion, RecommendedVersion);
		}

	};
}