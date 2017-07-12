//
// Created by svakhreev on 27.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERAWEBSOCKETSERVER_HPP
#define CAMERAMANAGERCORE_CAMERAWEBSOCKETSERVER_HPP

#include "boost/cerrno.hpp"
#include <memory>
#include <functional>
#include <simple-websocket-server/server_ws.hpp>
#include <nlohmann/json.hpp>

#include "../../model/CameraInformation.hpp"
#include "../../model/CameraLockFrame.hpp"
#include "../../signals/SignalContext.hpp"
#include "../model/SignalStatus.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

namespace cameramanagercore::schemas::ws
{

using cameramanagercore::model::CameraStatus;
using cameramanagercore::model::CameraAction;
using cameramanagercore::model::CameraInIdle;
using cameramanagercore::model::CameraLockFrame;

using namespace cameramanagercore::signals;

template <typename Contexts>
struct CameraWebSocketServer
{
    CameraWebSocketServer(Contexts contexts, int port, int thread_count)
        : _server(std::make_shared<WsServer>(port, thread_count)),
          _contexts(contexts),
          _signals_context(contexts.template Get<SignalsContext>())
    {
        _signals_context->AddSubscriber<CameraStatus>(CameraStatusDataProcess);
        _server->endpoint["^/camera_status/?$"];

        _signals_context->AddSubscriber<CameraAction>(CameraActionDataProcess);
        _server->endpoint["^/camera_action/?$"];

        _signals_context->AddSubscriber<CameraInIdle>(CameraInIdleDataProcess);
        _server->endpoint["^/camera_in_idle/?$"];

        _signals_context->AddSubscriber<CameraLockFrame>(CameraLockFrameDataProcess);
        _server->endpoint["^/camera_lock_frame/?$"];
    }

    ~CameraWebSocketServer()
    {
        _signals_context->RemoveSubscriber(CameraStatusDataProcess);
        _signals_context->RemoveSubscriber(CameraActionDataProcess);
        _signals_context->RemoveSubscriber(CameraInIdleDataProcess);
        _signals_context->RemoveSubscriber(CameraLockFrameDataProcess);
    }

    const std::function<void(SignalData<CameraStatus>)> CameraStatusDataProcess =
        [this] (SignalData<CameraStatus> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/camera_status/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_status/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_status/?$"].get_connections())
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

    const std::function<void(SignalData<CameraAction>)> CameraActionDataProcess =
        [this] (SignalData<CameraAction> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/camera_action/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_action/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_action/?$"].get_connections())
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

    const std::function<void(SignalData<CameraInIdle>)> CameraInIdleDataProcess =
        [this] (SignalData<CameraInIdle> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/camera_in_idle/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_in_idle/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_in_idle/?$"].get_connections())
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

    const std::function<void(SignalData<CameraLockFrame>)> CameraLockFrameDataProcess =
        [this] (SignalData<CameraLockFrame> signal)
    {
        switch (signal.status)
        {
            case DataStatus::Added:
            {
                for (auto& s: _server->endpoint["^/camera_lock_frame/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_lock_frame/?$"].get_connections())
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
                for (auto& s: _server->endpoint["^/camera_lock_frame/?$"].get_connections())
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

#endif //CAMERAMANAGERCORE_CAMERAWEBSOCKETSERVER_HPP
