#include "pch.h"
#include <CharacterEncoding.h>
namespace FFmpegInterop 
{

	Vector<CharacterEncoding^>^ FFmpegInterop::CharacterEncoding::internalMap;
	Windows::Foundation::Collections::IVectorView<CharacterEncoding^>^ FFmpegInterop::CharacterEncoding::internalView;
	std::mutex FFmpegInterop::CharacterEncoding::mutex;

}