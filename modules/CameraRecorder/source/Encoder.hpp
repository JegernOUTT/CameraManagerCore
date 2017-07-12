//
// Created by svakhreev on 07.04.17.
//

#ifndef CAMERARECORDER_ENCODER_HPP
#define CAMERARECORDER_ENCODER_HPP

#include <string>

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

#include <boost/circular_buffer.hpp>

#include "Decoder.hpp"

namespace camerarecorder
{

class Encoder
{
public:
    Encoder(Input input, DecoderInformation information);
    bool encode(std::string name, const std::vector<VideoFrame>& archived_frames);

private:
    Input _input;
    DecoderInformation _information;
};

}

#endif //CAMERARECORDER_ENCODER_HPP
