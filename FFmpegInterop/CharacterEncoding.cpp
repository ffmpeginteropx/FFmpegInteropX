#include "pch.h"
#include <CharacterEncoding.h>
namespace FFmpegInteropX 
{

	Vector<CharacterEncoding^>^ FFmpegInteropX::CharacterEncoding::internalMap;
	Windows::Foundation::Collections::IVectorView<CharacterEncoding^>^ FFmpegInteropX::CharacterEncoding::internalView;
	std::mutex FFmpegInteropX::CharacterEncoding::mutex;

}