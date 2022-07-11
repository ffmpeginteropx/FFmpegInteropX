#pragma once

namespace FFmpegInteropX
{
    class AttachedFile 
    {
    public:
        winrt::hstring Name();
        winrt::hstring MimeType();
        uint64_t Size();

    public:
        winrt::Windows::Storage::Streams::IBuffer GetBuffer();

        AttachedFile(winrt::hstring  const& name, winrt::hstring  const& mimeType, AVStream* stream)
        {
            this->name = name;
            this->mimeType = mimeType;
            this->stream = stream;
        }

    private:
        winrt::hstring name{};
        winrt::hstring mimeType{};

        AVStream* stream = NULL;
    };
}
