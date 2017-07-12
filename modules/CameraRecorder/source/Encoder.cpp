//
// Created by svakhreev on 07.04.17.
//

#include "Encoder.hpp"
#include <boost/filesystem.hpp>

namespace camerarecorder
{

Encoder::Encoder(Input input, DecoderInformation information)
    : _input(input),
      _information(information)
{
}

bool Encoder::encode(std::string name,
                     const std::vector<VideoFrame>& archived_frames)
{
    std::error_code error;

    OutputFormat  output_format;
    FormatContext format_context;

    output_format.setFormat("", name);
    format_context.setFormat(output_format);

    Codec codec = findEncodingCodec(output_format);
    Stream stream = format_context.addStream(codec);
    VideoEncoderContext encoder { stream };

    encoder.setWidth(_information.width);
    encoder.setHeight(_information.height);
    if (_information.pixel_format > -1)
        encoder.setPixelFormat(_information.pixel_format);
    encoder.setTimeBase(Rational{1, 1000});
    encoder.setBitRate(_information.bitrate);
    stream.setFrameRate(_information.frame_rate);

    boost::filesystem::path dir(_input.camera_information.archive_path);
    boost::filesystem::create_directory(dir);
    boost::filesystem::path file(name);
    format_context.openOutput((dir / file).string(), error);
    if (error)
    {
        std::cerr << "Can't open output\n";
        return false;
    }

    encoder.open(Codec{}, error);
    if (error)
    {
        std::cerr << "Can't open encoder\n";
        return false;
    }

    format_context.dump();
    format_context.writeHeader();
    format_context.flush();

    int64_t count = 0;
    int64_t increment_by = static_cast<int64_t>(encoder.timeBase().getDenominator() / _information.frame_rate());
    for (const auto& frame: archived_frames)
    {
        auto cloned_frame = frame;
        cloned_frame.setTimeBase(encoder.timeBase());
        cloned_frame.setPts(Timestamp{count, encoder.timeBase()});
        count += increment_by;

        Packet packet = cloned_frame ? encoder.encode(cloned_frame, error) : encoder.encode(error);
        if (error || !packet)
        {
            continue;
        }

        packet.setStreamIndex(0);
        format_context.writePacket(packet, error);
        if (error)
        {
            std::cerr << "Error write packet: " << error << ", " << error.message() << std::endl;
            continue;
        }

    }

    format_context.writeTrailer(error);
    return error ? false : true;
}


}