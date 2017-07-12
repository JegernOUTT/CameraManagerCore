//
// Created by svakhreev on 11.04.17.
//

#ifndef CAMERAMANAGERCORE_CAMERARECORDER_HPP
#define CAMERAMANAGERCORE_CAMERARECORDER_HPP


#include <memory>
#include <vector>
#include <iterator>
#include <sstream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <string_view>

#include <onvif_wrapper/Onvif.hpp>
#include <boost/format.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

#include "./extra/RecorderModel.hpp"
#include "../CameraContextVisitors.hpp"
#include "../../signals/SignalContext.hpp"
#include "../../utility/LoggerSettings.hpp"
#include "../../thread_pool/Misc.hpp"

namespace cameramanagercore::model
{

struct Camera;
struct EventArchiveSettings;

}

namespace cameramanagercore::cameras::processors
{

using namespace onvifwrapper;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::model;
using namespace std::string_literals;

namespace bp = boost::process;

template < typename Func >
class ProcessWrapper
{
public:
    ProcessWrapper(boost::filesystem::path exe_path, Func read_callback)
        : _process(exe_path, bp::std_in < _in, bp::std_out > _out),
          _read_callback(read_callback),
          _out_reader(&ProcessWrapper<Func>::Reader, this)
    {
    }

    ~ProcessWrapper()
    {
        if (_process.running())
            _process.terminate();
        else
            _process.join();

        if (_out_reader.joinable())
            _out_reader.join();
    }

    ProcessWrapper& operator<<(std::string_view data)
    {
        _in << boost::format("%1%\0") % data << std::endl;
        return *this;
    }

    void Wait()
    {
        _process.wait();
    }

private:
    void Reader()
    {
        std::string line;
        while (_process.running() && std::getline(_out, line) && !line.empty())
            _read_callback(line);
    }

    bp::opstream _in;
    bp::ipstream _out;
    bp::child _process;

    Func _read_callback;
    std::thread _out_reader;
};

template < typename SignalContext,
           typename CameraContext,
           typename ModelContext,
           typename EventContext,
           typename ThreadPool >
class CameraRecorderProcessor
{
    using CameraContextPtr = std::shared_ptr<CameraContext>;
    using ModelContextPtr  = std::shared_ptr<ModelContext>;
    using ThreadPoolPtr    = std::shared_ptr<ThreadPool>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;
    using Process = ProcessWrapper<std::function<void(string)>>;
    using EventContextPtr  = std::shared_ptr<EventContext>;

public:
    CameraRecorderProcessor(SignalContextPtr signal_context,
                            CameraContextPtr camera_context,
                            ModelContextPtr model_context,
                            EventContextPtr event_context,
                            ThreadPoolPtr thread_pool)
      : _signal_context(signal_context),
        _camera_context(camera_context),
        _model_context(model_context),
        _event_context(event_context),
        _thread_pool(thread_pool),
        _event_updater([&] (auto cd)
                       {
                           std::lock_guard<std::mutex> lg(_mu_model);
                           _eventsettings = _model_context->GetEventArchiveSettings();
                       }),
        _eventsettings(model_context->GetEventArchiveSettings())
    {
        _signal_context->template AddSubscriber<EventArchiveSettings>(_event_updater);
        Init();
    }

    ~CameraRecorderProcessor()
    {
        _signal_context->template RemoveSubscriber<EventArchiveSettings>(_event_updater);
        Stop();
    }

    void operator()() noexcept
    { }

private:
    void Init()
    {
        using namespace boost::process;
        using namespace cameramanagercore::cameras::processors::extra;

        std::vector<string> addresses;
        GetIpAddresses(std::back_inserter(addresses));
        if (addresses.size() == 0)
        {
            LOG(error) << "Can not retrieve any valid ip address of core";
            return;
        }
        auto settings = _model_context->GetSettings();
        string event_ws_url = (boost::format("%1%:%2%/events/")
                               % addresses.front()
                               % settings.front()->ws_events_port).str();
        LOG(debug) << boost::format("WS event url: %1%") % event_ws_url;

        for (const auto& s: _eventsettings)
        {
            auto maybe_camera = _model_context->FindCameraById(s->camera_id);
            if (!maybe_camera)
            {
                LOG(error) << boost::format("Can not find camera by given id: %1%") % s->camera_id;
                continue;
            }
            auto camera = maybe_camera.value();

            auto action = [&]
            {
                return ((*_camera_context) | camera) & std::make_tuple(GetStreamUri, ""s,
                                                                       StreamType::RTP_UNICAST,
                                                                       TransportProtocol::RTSP);
            };
            auto maybe_ptz_result = (*_thread_pool)
                                    | boost::packaged_task<std::optional<tuple<Result, MediaUri>>> { action }
                                    | cameramanagercore::thread_pool::execute;
            if (!maybe_ptz_result)
            {
                LOG(error) << "Error in request executing (task pool error)";
                return;
            }

            auto [ptz_result, media_uri] = maybe_ptz_result.value().value();
            if (ptz_result.is_error)
            {
                LOG(error) << boost::format("Can not get stream url for camera: %1%") % s->camera_id;
                continue;
            }

            string log_pass = (boost::format("%1%:%2%@") % camera->login % camera->password).str();
            auto stream_url = media_uri.uri.insert(7, log_pass);
            auto reader = [&] (string data)
            {
                try
                {
                    Output out = nlohmann::json::parse(data);
                    if (out.status == OperationStatus::Ok)
                    {
                        auto maybe_event = _event_context->FindEvent(out.event_hash);
                        if (!maybe_event) return;

                        auto [ok, changed_event] = _event_context->Change(maybe_event.value(),
                                                                          EventStatus::HasVideo,
                                                                          "");
                        if (ok)
                        {
                            LOG(debug) << boost::format("Success in event (%1%) status changed (has_video)")
                                          % changed_event->hash;
                        }
                        else
                        {
                            LOG(error) << boost::format("Event changing status (has_video) error");
                        }
                    }
                }
                catch (...)
                {
                    LOG(error) << boost::format("Event changing status (has_video) error");
                }
            };
            auto process = std::make_shared<Process>(boost::process::search_path("CameraRecorder",
                                                                                 { boost::filesystem::current_path() }),
                                                reader);

            nlohmann::json input = Input { { s->camera_id, s->event_categories,
                                               s->buffer_seconds, s->record_seconds },
                                           { stream_url, "quick_event/", event_ws_url } };
            (*process) << input.dump();
            _process_childs[s->camera_id] = process;
            LOG(debug) << boost::format("Starting process with input data: %1%") % input.dump();
        }
    }

    void Stop()
    {
        _process_childs.clear();
    }

    template <typename OutputIterator>
    void GetIpAddresses(OutputIterator out)
    {
        using namespace boost::asio;

        io_service io_service;
        ip::tcp::tcp::resolver resolver(io_service);
        ip::tcp::tcp::resolver::query query(boost::asio::ip::host_name(), "");
        ip::tcp::tcp::resolver::iterator iter = resolver.resolve(query);
        ip::tcp::tcp::resolver::iterator end;
        while (iter != end)
        {
            ip::tcp::tcp::endpoint ep = *iter++;
            *out++ = ep.address().to_string();
        }
    }

    SignalContextPtr _signal_context;
    CameraContextPtr _camera_context;
    ModelContextPtr _model_context;
    EventContextPtr _event_context;
    ThreadPoolPtr _thread_pool;

    std::function<void(SignalData<EventArchiveSettings>)> _event_updater;

    mutable std::mutex _mu_model;
    std::unordered_map<int, std::shared_ptr<Process>> _process_childs;
    std::vector<std::shared_ptr<EventArchiveSettings>> _eventsettings;
};

}

#endif //CAMERAMANAGERCORE_CAMERARECORDER_HPP
