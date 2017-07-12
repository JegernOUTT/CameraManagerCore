//
// Created by svakhreev on 11.04.17.
//

#ifndef CAMERARECORDER_EVENTHANDLER_HPP
#define CAMERARECORDER_EVENTHANDLER_HPP

#include <vector>
#include <algorithm>
#include <memory>
#include <simple-websocket-server/client_ws.hpp>


#include "Model.hpp"
#include "Encoder.hpp"

namespace camerarecorder
{

struct EventHandler
{
    using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

    static void Start(Input input, Decoder& decoder, Encoder& encoder);
};

}

#endif //CAMERARECORDER_EVENTHANDLER_HPP
