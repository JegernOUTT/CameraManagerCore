//
// Created by svakhreev on 07.04.17.
//

#ifndef CAMERARECORDER_DECODER_HPP
#define CAMERARECORDER_DECODER_HPP

#include <chrono>
#include <iostream>
#include <algorithm>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <experimental/optional>

#include <boost/circular_buffer.hpp>
#include <boost/format.hpp>

#include <avcpp/av.h>
#include <avcpp/avutils.h>
#include <avcpp/audioresampler.h>
#include <avcpp/codec.h>
#include <avcpp/codeccontext.h>
#include <avcpp/ffmpeg.h>
#include <avcpp/format.h>
#include <avcpp/formatcontext.h>
#include <avcpp/packet.h>
#include <avcpp/videorescaler.h>

#include "Model.hpp"

namespace camerarecorder
{

using namespace av;

struct DecoderInformation
{
    int width;
    int height;
    PixelFormat pixel_format;
    int bitrate;
    Rational frame_rate;
};

class Decoder
{
public:
    Decoder(Input input);

    ~Decoder();

    bool IsConnected() const noexcept;

    bool IsErrorWhileReading() const noexcept;

    void CopyBufferAndFillExtra(std::vector<VideoFrame>* archived) noexcept;

    void Reconnect() noexcept;

    std::experimental::optional<DecoderInformation>
    GetDecoderInformation() const noexcept;

private:
    void _Init() noexcept;
    void _Run() noexcept;
    void _FillExtraBuffer(const VideoFrame& frame) noexcept;

private:
    const size_t frame_delay = 61u;
    Input _input;
    boost::circular_buffer<VideoFrame> _buffer;
    std::atomic_bool _is_connected;

    std::atomic_bool _is_running;
    std::unique_ptr<std::thread> _decoder_thread;
    mutable std::mutex _decoder_mu;

    FormatContext _format_context;
    VideoDecoderContext _video_decoder_context;
    Stream _stream;
    std::size_t _video_stream_index;
    std::atomic_bool _error_read;

    std::vector<VideoFrame>* _extra_buffer;
};

}


#endif //CAMERARECORDER_DECODER_HPP
