//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_MODELWEBSOCKETSERVER_HPP
#define CAMERAMANAGERCORE_MODELWEBSOCKETSERVER_HPP

#include <memory>
#include <type_traits>
#include <functional>
#include <nlohmann/json.hpp>
#include <simple-websocket-server/server_ws.hpp>
#include <boost/cerrno.hpp>

#include "../model/SignalStatus.hpp"
#include "../../signals/SignalContext.hpp"
#include "../../utility/LoggerSettings.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

namespace cameramanagercore::schemas::ws
{

template < typename Contexts,
           typename... Types >
struct ModelWebSocketServer
{
    ModelWebSocketServer(Contexts contexts, int port, int thread_count)
        : _server(std::make_shared<WsServer>(port, thread_count)),
          _contexts(contexts),
          _signals_context(contexts.template Get<SignalsContext>())
    {
        auto func = [&] (auto&& value)
        {
            Process(value);
        };

        _callbacks = std::make_tuple(std::function<void(SignalData<Types>)>(func)...);

        ((_signals_context->AddSubscriber<Types>(std::get<std::function<void(SignalData<Types>)>>(_callbacks))), ...);
        _server->endpoint["^/model/?$"];
    }

    ~ModelWebSocketServer()
    {
        ((_signals_context->RemoveSubscriber<Types>(std::get<std::function<void(SignalData<Types>)>>(_callbacks))), ...);
    }

    std::tuple<std::function<void(SignalData<Types>)>...> _callbacks;


    template<typename T>
    void Process(SignalData<T> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/model/?$"].get_connections())
                {
                    auto send_stream = std::make_shared<WsServer::SendStream>();
                    nlohmann::json answer;
                    answer["status"] = "OnAdd";
                    answer["data"] = *signal.data;

                    *send_stream << answer;

                    _server->send(s, send_stream, [](const boost::system::error_code&) { });
                }
            }
            break;

            case DataStatus::Changed:
            {
                for (auto& s: _server->endpoint["^/model/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/model/?$"].get_connections())
                {
                    auto send_stream = std::make_shared<WsServer::SendStream>();

                    nlohmann::json answer;
                    answer["status"] = "OnRemove";
                    answer["data"] = *signal.data;

                    *send_stream << answer;
                    _server->send(s, send_stream, [](const boost::system::error_code&) { });
                }
            }
            break;

            default: break;
        }
    }

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

#endif //CAMERAMANAGERCORE_MODELWEBSOCKETSERVER_HPP
