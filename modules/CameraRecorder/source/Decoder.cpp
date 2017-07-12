//
// Created by svakhreev on 07.04.17.
//

#include "Decoder.hpp"

namespace camerarecorder
{

Decoder::Decoder(Input input) :
    _input(input),
    _is_connected(false),
    _is_running(false),
    _decoder_thread(std::make_unique<std::thread>(&Decoder::_Init, this)),
    _error_read(false)
{
    size_t buffered_frames_count = static_cast<size_t>(_input.event_settings.buffer_time
                                                       * _stream.frameRate().getNumerator()) + frame_delay;
    _buffer.set_capacity(buffered_frames_count);
}

Decoder::~Decoder()
{
    _is_running = false;
    if (_decoder_thread && _decoder_thread->joinable())
        _decoder_thread->join();
}

bool Decoder::IsConnected() const noexcept
{
    return _is_connected;
}

bool Decoder::IsErrorWhileReading() const noexcept
{
    return _error_read;
}

void Decoder::Reconnect() noexcept
{
    _is_running = false;
    if (_decoder_thread && _decoder_thread->joinable())
        _decoder_thread->join();
    _decoder_thread = std::make_unique<std::thread>(&Decoder::_Init, this);
}

std::experimental::optional<DecoderInformation>
Decoder::GetDecoderInformation() const noexcept
{
    if (!_is_connected)
    {
        return {};
    }
    return DecoderInformation{ _video_decoder_context.width(),
                               _video_decoder_context.height(),
                               _video_decoder_context.pixelFormat(),
                               _video_decoder_context.bitRate(),
                               _stream.frameRate() };
}

void
Decoder::CopyBufferAndFillExtra(std::vector<VideoFrame>* archived) noexcept
{
    std::unique_lock<std::mutex> ul(_decoder_mu);
    size_t buffered_frames_count = static_cast<size_t>(_input.event_settings.buffer_time
                                                       * _stream.frameRate().getNumerator()) + frame_delay;
    size_t recorded_frames_count = static_cast<size_t>(_input.event_settings.record_time
                                                       * _stream.frameRate().getNumerator());
    archived->clear();
    archived->reserve(buffered_frames_count + recorded_frames_count);

    archived->insert(archived->begin(), _buffer.begin(), _buffer.end());

    _extra_buffer = archived;
    ul.unlock();

    size_t complete_frames_count = _buffer.size() + recorded_frames_count;
    while (_extra_buffer->size() < complete_frames_count)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    _extra_buffer = nullptr;
}

void Decoder::_Init() noexcept
{
    _is_running = true;

    std::error_code error;

    while (!_is_connected)
    {
        try
        {
            _format_context.openInput(_input.camera_information.stream_url, error);
            if (error)
            {
                std::cerr << boost::format("Can't open input url %1%: %2%\n") % _input.camera_information.stream_url
                             % error;
                continue;
            }

            _format_context.findStreamInfo(error);
            if (error)
            {
                std::cerr << boost::format("Stream opened, but can not retrieve stream info: %2%\n") % error;
                continue;
            }

            for (size_t i = 0; i < _format_context.streamsCount(); ++i)
            {
                if (_format_context.stream(i).mediaType() == AVMEDIA_TYPE_VIDEO)
                {
                    _video_stream_index = i;
                    _stream = _format_context.stream(i);
                    break;
                }
            }

            if (_stream.isNull())
            {
                std::cerr << "Video stream not found\n";
                continue;
            }

            if (_stream.isValid())
            {
                _video_decoder_context = VideoDecoderContext(_stream);
                _video_decoder_context.setRefCountedFrames(true);

                _video_decoder_context.open(Codec(), error);
                if (error)
                {
                    std::cerr << "Can not open codec\n";
                    continue;
                }
            }

            _is_connected = true;
        }
        catch (...)
        {
            std::cerr << "Unexpected error in initialization. Trying again...\n";
            continue;
        }
    }

    size_t frames_count = static_cast<size_t>(_input.event_settings.buffer_time
                                              * _stream.frameRate().getNumerator()) + frame_delay;
    _buffer.set_capacity(frames_count);
    _Run();
}

void Decoder::_FillExtraBuffer(const VideoFrame& frame) noexcept
{
    size_t recorded_frames_count = static_cast<size_t>(_input.event_settings.record_time
                                                       * _stream.frameRate().getNumerator());
    size_t complete_frames_count = _buffer.size() + recorded_frames_count;
    if (_extra_buffer && _extra_buffer->size() < complete_frames_count)
        _extra_buffer->push_back(frame);
}

void Decoder::_Run() noexcept
{
    std::error_code error;

    while (_is_running)
    {
        try
        {
            Packet packet = _format_context.readPacket(error);
            if (error)
            {
                std::cerr << boost::format("Packet reading error: %1%\n") % error;
                _error_read = true;
                continue;
            }

            if (!packet) { _error_read = true; continue; }
            if (packet && packet.streamIndex() != _video_stream_index) { _error_read = true; continue; }

            auto frame = _video_decoder_context.decode(packet, error);
            if (error)
            {
                _error_read = true;
                std::cerr << boost::format("Decoding error: %1%\n") % error;
                continue;
            }

            if (!frame)
            {
                // Empty frame (It may be B-frame, continue and waiting non empty frame)
                _error_read = false;
                continue;
            }

            if (frame)
            {
                _error_read = false;
                frame.setStreamIndex(0);
                frame.setPictureType();

                std::unique_lock<std::mutex> ul(_decoder_mu);
                _buffer.push_back(frame);
                _FillExtraBuffer(frame);
                ul.unlock();
            }
        }
        catch (...)
        {
            std::cerr << "Unexpected error in decoding. Trying again\n";
            continue;
        }
    }
}



}