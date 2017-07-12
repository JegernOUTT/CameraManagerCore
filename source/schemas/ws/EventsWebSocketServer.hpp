//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_EVENTSWEBSOCKETSERVER_HPP
#define CAMERAMANAGERCORE_EVENTSWEBSOCKETSERVER_HPP

#include "boost/cerrno.hpp"
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
#include <simple-websocket-server/server_ws.hpp>

#include "../../model/Event.hpp"
#include "../../signals/SignalContext.hpp"
#include "../model/SignalStatus.hpp"

namespace cameramanagercore::schemas::ws
{

using cameramanagercore::model::Event;

template <typename Contexts>
struct EventWebSocketServer
{
    EventWebSocketServer(Contexts contexts, int port, int thread_count)
       : _server(std::make_shared<WsServer>(port, thread_count)),
         _contexts(contexts),
         _signals_context(contexts.template Get<SignalsContext>())
    {
        _signals_context->AddSubscriber<Event>(EventsDataProcess);
        _server->endpoint["^/events/?$"];
    }

    ~EventWebSocketServer()
    {
        _signals_context->RemoveSubscriber(EventsDataProcess);
    }

    const std::function<void(SignalData<Event>)> EventsDataProcess =
        [this] (SignalData<Event> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/events/?$"].get_connections())
                {
                    auto send_stream = std::make_shared<WsServer::SendStream>();

                    nlohmann::json answer;
                    answer["status"] = "OnAdd";
                    answer["data"] = *signal.data;

                    *send_stream << answer;
                    _server->send(s, send_stream, [](const boost::system::error_code& ec) { });
                }
            }
                break;

            case DataStatus::Changed:
            {
                for (auto& s: _server->endpoint["^/events/?$"].get_connections())
                {
                    auto send_stream = std::make_shared<WsServer::SendStream>();

                    nlohmann::json answer;
                    answer["status"] = "OnChange";
                    answer["data"] = *signal.data;

                    *send_stream << answer;
                    _server->send(s, send_stream, [](const boost::system::error_code& ec) { });
                }
            }
                break;

            case DataStatus::Removed:
            {
                for (auto& s: _server->endpoint["^/events/?$"].get_connections())
                {
                    auto send_stream = std::make_shared<WsServer::SendStream>();

                    nlohmann::json answer;
                    answer["status"] = "OnRemove";
                    answer["data"] = *signal.data;

                    *send_stream << answer;
                    _server->send(s, send_stream, [](const boost::system::error_code& ec) { });
                }
            }
                break;

            default: break;
        }
    };

    void Start()
    {
        _server->start();
    }

    void Stop()
    {
        _server->stop();
    }

private:
    std::shared_ptr<WsServer> _server;
    Contexts _contexts;
    std::shared_ptr<SignalsContext> _signals_context;
};

}

#endif //CAMERAMANAGERCORE_EVENTSWEBSOCKETSERVER_HPP
