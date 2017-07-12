//
// Created by svakhreev on 11.04.17.
//

#include "EventHandler.hpp"

#include <nlohmann/json.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace camerarecorder
{
    void EventHandler::Start(Input input, Decoder& decoder, Encoder& encoder)
    {
        using namespace nlohmann;

        std::vector<std::string> allowed_categories;
        boost::split(allowed_categories,
                     input.event_settings.event_categories,
                     boost::is_any_of(";"));
        std::sort(allowed_categories.begin(), allowed_categories.end());

        WsClient client(input.camera_information.websocket_url);

        client.on_message = [&](std::shared_ptr<WsClient::Message> message)
        {
            SignalEvent signal_event = json::parse(message->string());

            if (signal_event.status == SignalStatus::OnChange
                && input.event_settings.camera_id == signal_event.data.camera_id
                && (signal_event.data.processed && !signal_event.data.acknowledged))
            {
                std::vector<std::string> event_categories;
                std::vector<std::string> intersections;
                boost::split(event_categories,
                             signal_event.data.category,
                             boost::is_any_of(";"));
                std::sort(event_categories.begin(), event_categories.end());
                std::set_intersection(allowed_categories.begin(), allowed_categories.end(),
                                      event_categories.begin(),   event_categories.end(),
                                      std::back_inserter(intersections));
                if (intersections.size() == 0) return;

                std::cerr << boost::format("Started recording event with hash: %1%")
                             % signal_event.data.hash << std::endl;
                std::vector<VideoFrame> archived;
                decoder.CopyBufferAndFillExtra(&archived);
                auto success = encoder.encode((boost::format("%1%.mp4") % signal_event.data.hash).str(),
                                              archived);
                std::cerr << boost::format("Finished recording event with hash: %1%")
                             % signal_event.data.hash << std::endl;

                if (success)
                    std::cout << json(Output{ signal_event.data.hash, OperationStatus::Ok }) << std::endl;
                else
                    std::cout << json(Output{ signal_event.data.hash, OperationStatus::Failed }) << std::endl;
            }
        };

        client.on_open = [&]()
        {
            std::cerr << "Connection to core established" << std::endl;
        };

        client.on_close = [](int status, const std::string& reason)
        {
            std::cerr << "Connection has lost: " << status << std::endl;
            exit(-1);
        };

        client.on_error = [](const boost::system::error_code& ec)
        {
            std::cerr << "Client: Error: " << ec << ", error message: " << ec.message() << std::endl;
            exit(-1);
        };

        client.start();
    }
}