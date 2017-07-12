#include <iostream>
#include <set>
#include <map>
#include <memory>
#include <fstream>
#include <functional>

#include <avcpp/av.h>
#include <avcpp/ffmpeg.h>
#include <avcpp/codec.h>
#include <avcpp/packet.h>
#include <avcpp/videorescaler.h>
#include <avcpp/audioresampler.h>
#include <avcpp/avutils.h>

// API2
#include <avcpp/format.h>
#include <avcpp/formatcontext.h>
#include <avcpp/codec.h>
#include <avcpp/codeccontext.h>

#include <nlohmann/json.hpp>

#include <boost/circular_buffer.hpp>
#include <boost/stacktrace.hpp>

#include "source/EventHandler.hpp"

using namespace std;
using namespace av;
using namespace camerarecorder;

void my_terminate_handler()
{
    std::ofstream error_f("stacktrace.log");
    std::cerr << "Terminate called:\n"
              << boost::stacktrace::stacktrace()
              << std::endl;
    error_f << "Terminate called:\n"
            << boost::stacktrace::stacktrace()
            << std::endl;
    std::abort();
}

// Input input { EventSettings { 1, "all", 10, 10 },
//CameraInformation { "rtsp://admin:admin@10.10.20.21/onvif-media/media.amp?profile=profile_1_h264",
//"archive/",
//"10.10.20.2:8087/events/" } };

int main(int argc, char **argv)
{
    av::init();
    av::setFFmpegLoggingLevel(AV_LOG_ERROR);
    std::set_terminate(&my_terminate_handler);

    Input input;
    try
    {
        string data;
        std::cin >> data;
        input = nlohmann::json::parse(data);
    }
    catch (...)
    {
        std::cerr << "Unable to parse input parameters" << std::endl;
        exit(-1);
    }

//     input = Input { EventSettings { 1, "all", 10, 10 },
//        CameraInformation { "rtsp://admin:admin@10.10.20.21/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast",
//                            "quick_event/",
//                            "127.0.0.1:8088/events/" } };

    std::cerr << "Input parameters: " << nlohmann::json(input) << std::endl;

    Decoder decoder(input);
    std::this_thread::sleep_for(5s);
    while (!decoder.IsConnected())
    {
        std::cout << nlohmann::json(Output{ "", OperationStatus::ConnectionFailed }) << std::endl;
        std::this_thread::sleep_for(1s);
    }

    Encoder encoder(input, decoder.GetDecoderInformation().value());
    EventHandler::Start(input, decoder, encoder);

    return 0;
}
