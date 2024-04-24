#pragma once

#include "FFmpegTranscodeOutput.g.h"
#include "FFmpegTranscodeInputCropRectangle.g.h"
#include "FFmpegTranscodeInputCropEntry.g.h"
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

    struct FFmpegTranscodeInputCropEntry : FFmpegTranscodeInputCropEntryT<FFmpegTranscodeInputCropEntry>
    {
        FFmpegInteropX::FFmpegTranscodeInputCropRectangle CropRectangle() const { return crop_rectangle; }
        void CropRectangle(FFmpegInteropX::FFmpegTranscodeInputCropRectangle const& value) { crop_rectangle = value; }

        uint64_t FrameNumber() const { return frame_number; }
        void FrameNumber(uint64_t const value) { frame_number = value; }

        FFmpegTranscodeInputCropEntry() { }
        FFmpegTranscodeInputCropEntry(FFmpegInteropX::FFmpegTranscodeInputCropRectangle crop_rectangle, uint64_t frame_number)
        {
            this->crop_rectangle = crop_rectangle;
            this->frame_number = frame_number;
        }
    private:
        FFmpegInteropX::FFmpegTranscodeInputCropRectangle crop_rectangle = {};
        uint64_t frame_number = {};
    };

    struct FFmpegTranscodeInput : FFmpegTranscodeInputT<FFmpegTranscodeInput>
    {
        hstring FileName() const { return filename; }
        void FileName(hstring const& value) { filename = value; }

        int VideoStreamIndex() const { return video_stream_index; }
        void VideoStreamIndex(int const value) { video_stream_index = value; }

        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropEntry> CropEntries() const { return crop_entries; }
        void CropEntries(Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropEntry> const& value) { crop_entries = value; }

        FFmpegTranscodeInput() { }
        FFmpegTranscodeInput(hstring const& input, int const video_stream_index,
            Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropEntry> crop_entries)
        {
            this->filename = input;
            this->video_stream_index = video_stream_index;
            this->crop_entries = crop_entries;
        }

    private:
        hstring filename;
        int video_stream_index = {};
        Windows::Foundation::Collections::IVectorView<FFmpegInteropX::FFmpegTranscodeInputCropEntry> crop_entries = {};
    };

    struct FFmpegTranscodeOutput : FFmpegTranscodeOutputT<FFmpegTranscodeOutput>
    {
        hstring FileName() const { return filename; }
        void FileName(hstring const& value) { filename = value; }

        OutputType Type() const { return type; }
        void Type(OutputType const value) { type = value; }

        uint32_t CRF() const { return crf; }
        void CRF(uint32_t const value) { crf = value; }

        OutputPresetType Preset() const { return preset; }
        void Preset(OutputPresetType const value) { preset = value; }

        Size PixelSize() const { return pixelSize; }
        void PixelSize(Size const value) { pixelSize = value; }

        double FrameRate() const { return frameRate; }
        void FrameRate(double const value) { frameRate = value; }

        FFmpegTranscodeOutput() { }

    private:
        hstring filename;
        OutputType type = {};
        uint32_t crf = {};
        OutputPresetType preset = {};
        Size pixelSize = {};
        double frameRate = {};
    };

    struct FFmpegTranscode : FFmpegTranscodeT<FFmpegTranscode>
    {
        FFmpegTranscode() { }
        void Run(FFmpegInteropX::FFmpegTranscodeInput const& input, FFmpegInteropX::FFmpegTranscodeOutput const& output);

        virtual ~FFmpegTranscode();
        void Close();
    };
};

namespace winrt::FFmpegInteropX::factory_implementation
{
    struct FFmpegTranscodeInputCropEntry : FFmpegTranscodeInputCropEntryT<FFmpegTranscodeInputCropEntry, implementation::FFmpegTranscodeInputCropEntry>
    {
    };

    struct FFmpegTranscodeInputCropRectangle : FFmpegTranscodeInputCropRectangleT<FFmpegTranscodeInputCropRectangle, implementation::FFmpegTranscodeInputCropRectangle>
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
