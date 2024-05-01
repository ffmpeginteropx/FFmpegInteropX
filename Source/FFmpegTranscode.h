#pragma once

#include "FFmpegTranscodeOutput.g.h"
#include "FFmpegTranscodeInputCropRectangle.g.h"
#include "FFmpegTranscodeInputCropFrameEntry.g.h"
#include "FFmpegTranscodeInputTrimmingMarkerEntry.g.h"
#include "FFmpegTranscodeInput.g.h"
#include "FFmpegTranscode.g.h"
#include "pch.h"

namespace winrt::FFmpegInteropX::implementation
{
    struct FFmpegTranscodeInputCropRectangle : FFmpegTranscodeInputCropRectangleT<FFmpegTranscodeInputCropRectangle>
    {
        int CenterX() const { return center_x; }
        void CenterX(int const value) { center_x = value; }

        int CenterY() const { return center_y; }
        void CenterY(int const value) { center_y = value; }

        int Width() const { return width; }
        void Width(int const value) { width = value; }

        int Height() const { return height; }
        void Height(int const value) { height = value; }

        FFmpegTranscodeInputCropRectangle() { }
        FFmpegTranscodeInputCropRectangle(int center_x, int center_y, int width, int height)
        {
            this->center_x = center_x;
            this->center_y = center_y;
            this->width = width;
            this->height = height;
        }

    private:
        int center_x = {};
        int center_y = {};
        int width = {};
        int height = {};
    };

    struct FFmpegTranscodeInputCropFrameEntry : FFmpegTranscodeInputCropFrameEntryT<FFmpegTranscodeInputCropFrameEntry>
    {
        FFmpegInteropX::FFmpegTranscodeInputCropRectangle CropRectangle() const { return crop_rectangle; }
        void CropRectangle(FFmpegInteropX::FFmpegTranscodeInputCropRectangle const& value) { crop_rectangle = value; }

        int64_t FrameNumber() const { return frame_number; }
        void FrameNumber(int64_t const value) { frame_number = value; }

        FFmpegTranscodeInputCropFrameEntry() { }
        FFmpegTranscodeInputCropFrameEntry(int64_t frame_number, FFmpegInteropX::FFmpegTranscodeInputCropRectangle crop_rectangle)
        {
            this->crop_rectangle = crop_rectangle;
            this->frame_number = frame_number;
        }
    private:
        FFmpegInteropX::FFmpegTranscodeInputCropRectangle crop_rectangle = {};
        int64_t frame_number = {};
    };

    struct FFmpegTranscodeInputTrimmingMarkerEntry : FFmpegTranscodeInputTrimmingMarkerEntryT<FFmpegTranscodeInputTrimmingMarkerEntry>
    {
        bool TrimAfter() const { return trim_after; }
        void TrimAfter(bool const value) { trim_after = value; }

        int64_t FrameNumber() const { return frame_number; }
        void FrameNumber(int64_t const value) { frame_number = value; }

        FFmpegTranscodeInputTrimmingMarkerEntry() { }
        FFmpegTranscodeInputTrimmingMarkerEntry(int64_t frame_number, bool trim_after)
        {
            this->trim_after = trim_after;
            this->frame_number = frame_number;
        }
    private:
        bool trim_after = {};
        int64_t frame_number = {};
    };

    struct FFmpegTranscodeInput : FFmpegTranscodeInputT<FFmpegTranscodeInput>
    {
        hstring FileName() const { return filename; }
        void FileName(hstring const& value) { filename = value; }

        int VideoStreamIndex() const { return video_stream_index; }
        void VideoStreamIndex(int const value) { video_stream_index = value; }

        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropFrameEntry> CropFrames() const { return crop_frames; }
        void CropFrames(Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropFrameEntry> const& value) { crop_frames = value; }

        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputTrimmingMarkerEntry> TrimmingMarkers() const { return trimming_markers; }
        void TrimmingMarkers(Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputTrimmingMarkerEntry> const& value) { trimming_markers = value; }

        FFmpegTranscodeInput() { }
        FFmpegTranscodeInput(hstring const& input, int const video_stream_index,
            Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropFrameEntry> crop_frames,
            Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputTrimmingMarkerEntry> trimming_markers)
        {
            this->filename = input;
            this->video_stream_index = video_stream_index;
            this->crop_frames = crop_frames;
            this->trimming_markers = trimming_markers;
        }

    private:
        hstring filename;
        int video_stream_index = {};
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropFrameEntry> crop_frames;
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputTrimmingMarkerEntry> trimming_markers;
    };

    struct FFmpegTranscodeOutput : FFmpegTranscodeOutputT<FFmpegTranscodeOutput>
    {
        hstring FileName() const { return filename; }
        OutputType Type() const { return type; }
        uint32_t CRF() const { return crf; }
        OutputPresetType Preset() const { return preset; }
        Size PixelSize() const { return pixelSize; }
        double FrameRateMultiplier() const { return frameRateMultiplier; }

        FFmpegTranscodeOutput(hstring const& FileName, OutputType Type, uint32_t CRF, double FrameRateMultiplier,
            Size const& PixelSize, OutputPresetType Preset)
            : filename(FileName), type(Type), crf(CRF), frameRateMultiplier(FrameRateMultiplier), pixelSize(PixelSize), preset(Preset)
        {}

    private:
        hstring filename;
        OutputType type;
        uint32_t crf;
        OutputPresetType preset;
        Size pixelSize;
        double frameRateMultiplier;
    };

    struct FFmpegTranscode : FFmpegTranscodeT<FFmpegTranscode>
    {
        FFmpegTranscode()
        {
        }

        void Run(FFmpegInteropX::FFmpegTranscodeInput const& input, FFmpegInteropX::FFmpegTranscodeOutput const& output);

        winrt::event_token FrameOutputProgress(EventHandler<std::uint64_t> const& handler) { return frameOutputProgress.add(handler); }
        void FrameOutputProgress(winrt::event_token const& token) noexcept { frameOutputProgress.remove(token); }

        virtual ~FFmpegTranscode();
        void Close();

    private:
        void throw_av_error(int ret);
        void SetEncodingParameters(AVCodecContext& ctx, FFmpegInteropX::FFmpegTranscodeOutput const& output);

        int FilterWriteFrame(AVFrame& filteredFrame, int64_t skippedPts,
            AVFormatContext& outputFormatContext, AVCodecContext& outputCodecContext, AVPacket& outputPacket, bool flush);

        winrt::event<EventHandler<std::uint64_t>> frameOutputProgress;
    };
};

namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FFmpegTranscodeInputCropFrameEntry : FFmpegTranscodeInputCropFrameEntryT<FFmpegTranscodeInputCropFrameEntry, implementation::FFmpegTranscodeInputCropFrameEntry>
    {
    };

    struct FFmpegTranscodeInputCropRectangle : FFmpegTranscodeInputCropRectangleT<FFmpegTranscodeInputCropRectangle, implementation::FFmpegTranscodeInputCropRectangle>
    {
    };

    struct FFmpegTranscodeInputTrimmingMarkerEntry : FFmpegTranscodeInputTrimmingMarkerEntryT<FFmpegTranscodeInputTrimmingMarkerEntry, implementation::FFmpegTranscodeInputTrimmingMarkerEntry>
    {
    };

    struct FFmpegTranscodeInput : FFmpegTranscodeInputT<FFmpegTranscodeInput, implementation::FFmpegTranscodeInput>
    {
    };

    struct FFmpegTranscodeOutput : FFmpegTranscodeOutputT<FFmpegTranscodeOutput, implementation::FFmpegTranscodeOutput>
    {
    };

    struct FFmpegTranscode : FFmpegTranscodeT<FFmpegTranscode, implementation::FFmpegTranscode>
    {
    };
}
