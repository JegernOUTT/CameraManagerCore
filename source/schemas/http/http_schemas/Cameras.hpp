//
// Created by svakhreev on 28.03.17.
//

#ifndef CAMERAMANAGERCORE_CAMERAS_HPP
#define CAMERAMANAGERCORE_CAMERAS_HPP

#include <memory>
#include <chrono>
#include <exception>

#include <simple-web-server/server_http.hpp>
#include <nlohmann/json.hpp>
#include <onvif_wrapper/OnvifPod.hpp>
#include <boost/thread.hpp>

#include "../../decorators/SessionCheckerDecorator.hpp"
#include "../../decorators/CatchErrorDecorator.hpp"
#include "../../decorators/PermissionCheckerDecorator.hpp"
#include "../../decorators/CorsDecorator.hpp"
#include "../../details/InputParameters.hpp"
#include "../../details/ParseFromUrl.hpp"
#include "../../details/Utility.hpp"
#include "../../model/OperationResponse.hpp"
#include "../../model/Helpers.hpp"
#include "../../../session/SessionContext.hpp"
#include "../../../model/User.hpp"
#include "../../../thread_pool/ThreadPoolContext.hpp"
#include "../../../thread_pool/Misc.hpp"
#include "../../../cameras/CamerasContext.hpp"
#include "../../../cameras/CameraContextVisitors.hpp"

namespace cameramanagercore::schemas::http
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template<typename C>
struct CamerasSchema
{
    template<typename Contexts>
    static void CreateSchemas(Contexts contexts, std::shared_ptr<HttpServer> _server)
    {
        using namespace cameramanagercore::schemas::decorators;

        /* ptz */
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/relative_move\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { RelativeMoveSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/continuous_move\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { ContinuousMoveSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/absolute_move\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { AbsoluteMoveSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/stop\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { StopSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/get_presets\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetPresetsSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/go_to_preset\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { GoToPresetSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/change_preset\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { ChangePresetSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/add_preset\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { AddPresetSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/remove_preset\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { RemovePresetSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/go_to_home\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { GoToHomeSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/set_home\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { SetHomeSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/get_status\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetStatusSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/get_capabilities\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetCapabilitiesSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/ptz/get_nodes\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetNodesSc(contexts, res, req); }, contexts);


        /* device */
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/device/camera_reboot\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { CameraRebootSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/device/get_device_information\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetDeviceInformationSc(contexts, res, req); }, contexts);


        /* media */
        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/media/get_media_profiles\\??(.*)?$", {"GET", "OPTIONS"},
                      [=] (auto res, auto req) { GetMediaProfilesSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/media/get_stream_uri\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { GetStreamUriSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/media/start_multicast\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { StartMulticastSc(contexts, res, req); }, contexts);

        set_resource < PermissionChecker, SessionChecker, CorsChecker, ErrorCatcher >
                     (_server, "^/camera/media/stop_multicast\\??(.*)?$", {"POST", "OPTIONS"},
                      [=] (auto res, auto req) { StopMulticastSc(contexts, res, req); }, contexts);

        /* extra */
    }

    /* ptz */
    template < typename Contexts >
    static void RelativeMoveSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request relative move schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        PTZVector translation;
        PTZSpeed speed;
        try
        {
            auto [error, maybe_j] = get_from_body(request);
            if (!maybe_j)
            {
                LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % error;
                make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % error, 400);
                return;
            }

            translation = maybe_j.value()["translation"];
            speed = maybe_j.value()["speed"];
        }
        catch (...)
        {
            LOG(error) << "Can not find valid input parameters for this schema: Exta params error";
            make_4xx(response, "Can not find valid input parameters for this schema: Exta params error", 400);
            return;
        }
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context, model_context = model_context]
        {
            return (*cameras_context) | camera | std::make_tuple(RelativeMove, session_hash,
                                                                 model_context, translation, speed);
        };

        auto maybe_ptz_result =
        (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void ContinuousMoveSc(Contexts contexts,
                                 std::shared_ptr<Response> response,
                                 std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request continuous move schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        PTZSpeed speed;
        int timeout;
        try
        {
            auto [error, maybe_j] = get_from_body(request);
            if (!maybe_j)
            {
                LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % error;
                make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % error, 400);
                return;
            }

            speed = maybe_j.value()["speed"];
            timeout = maybe_j.value()["timeout"];
        }
        catch (...)
        {
            LOG(error) << "Can not find valid input parameters for this schema: Exta params error";
            make_4xx(response, "Can not find valid input parameters for this schema: Exta params error", 400);
            return;
        }
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context, model_context = model_context]
        {
            return (*cameras_context) | camera | std::make_tuple(ContinuousMove, session_hash,
                                                                 model_context, speed, timeout);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void AbsoluteMoveSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request absolute move schema";

        auto [signal_context, model_context, event_context,
                 cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        PTZSpeed speed;
        PTZVector position;
        try
        {
            auto [error, maybe_j] = get_from_body(request);
            if (!maybe_j)
            {
                LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % error;
                make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % error, 400);
                return;
            }

            speed = maybe_j.value()["speed"];
            position = maybe_j.value()["position"];
        }
        catch (...)
        {
            LOG(error) << "Can not find valid input parameters for this schema: Exta params error";
            make_4xx(response, "Can not find valid input parameters for this schema: Exta params error", 400);
            return;
        }
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context, model_context = model_context]
        {
            return (*cameras_context) | camera | std::make_tuple(AbsoluteMove, session_hash,
                                                                 model_context, speed, position);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void StopSc(Contexts contexts,
                       std::shared_ptr<Response> response,
                       std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request stop move schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int, bool, bool>(ParseFromUrl(request->path_match[1]), "camera_id",
                                                                    "pan_tilt", "zoom");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id, pan_tilt, zoom] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, pan_tilt = pan_tilt, zoom = zoom,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(Stop, session_hash, pan_tilt, zoom);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void GetPresetsSc(Contexts contexts,
                             std::shared_ptr<Response> response,
                             std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get presets schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetPresets, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<tuple<Result, vector<Preset>>>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, presets] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(presets));
    }

    template < typename Contexts >
    static void GoToPresetSc(Contexts contexts,
                             std::shared_ptr<Response> response,
                             std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request go to preset schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int, string>(ParseFromUrl(request->path_match[1]), "camera_id",
                                                                "preset_token");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id, preset_token] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, preset_token = preset_token, model_context = model_context,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GoToPreset, session_hash, model_context, preset_token);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void ChangePresetSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;
        using namespace ranges;

        LOG(trace) << "Request change preset schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int, string, string>(ParseFromUrl(request->path_match[1]), "camera_id",
                                                                        "preset_token", "new_preset_name");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id, preset_token, new_preset_name] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, preset_token = preset_token, new_preset_name = new_preset_name,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(ChangePreset, session_hash, preset_token, new_preset_name);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<tuple<Result, string>>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, changed_preset_token] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        // Presets request
        auto action_2 = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetPresets, session_hash);
        };

        auto maybe_ptz_result_ = (*camera_pool)
                                 | boost::packaged_task<tuple<bool, std::optional<tuple<Result, vector<Preset>>>>> { action_2 }
                                 | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result_)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked_, data_] = maybe_ptz_result_.value();
        if (locked_)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result_, presets] = data_.value();
        if (ptz_result_.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result_));
            return;
        }

        vector<Preset> filtered =
            presets | view::filter([t = changed_preset_token] (auto& preset) { return preset.token == t; });
        if (filtered.size() != 1)
        {
            LOG(error) << "Some error during presets request. Cannot find right preset";
            make_5xx(response, "Some error during presets request. Cannot find right preset", 500);
            return;
        }

        make_200(response, nlohmann::json(filtered.front()));
    }

    template < typename Contexts >
    static void AddPresetSc(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request add preset schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int, string>(ParseFromUrl(request->path_match[1]), "camera_id",
                                                                "preset_name");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id, preset_name] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, preset_name = preset_name,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(AddPreset, session_hash, preset_name);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<tuple<Result, string>>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, added_preset_token] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        // Presets request
        auto action_2 = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetPresets, session_hash);
        };

        auto maybe_ptz_result_ = (*camera_pool)
                                 | boost::packaged_task<tuple<bool, std::optional<tuple<Result, vector<Preset>>>>> { action_2 }
                                 | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result_)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked_, data_] = maybe_ptz_result_.value();
        if (locked_)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result_, presets] = data_.value();
        if (ptz_result_.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result_));
            return;
        }

        vector<Preset> filtered =
            presets | view::filter([t = added_preset_token] (auto& preset) { return preset.token == t; });
        if (filtered.size() != 1)
        {
            LOG(error) << "Some error during presets request. Cannot find right preset";
            make_5xx(response, "Some error during presets request. Cannot find right preset", 500);
            return;
        }

        make_200(response, nlohmann::json(filtered.front()));
    }

    template < typename Contexts >
    static void RemovePresetSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request to remove preset schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int, string>(ParseFromUrl(request->path_match[1]), "camera_id",
                                                                "preset_token");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id, preset_token] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, preset_token = preset_token,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(RemovePreset, session_hash, preset_token);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void GoToHomeSc(Contexts contexts,
                           std::shared_ptr<Response> response,
                           std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request go to home schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, model_context = model_context,
                       session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GoToHome, session_hash, model_context);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void SetHomeSc(Contexts contexts,
                          std::shared_ptr<Response> response,
                          std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request set home schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(SetHome, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void GetStatusSc(Contexts contexts,
                            std::shared_ptr<Response> response,
                            std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get status schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetStatus, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<tuple<Result, PTZStatus>>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, status] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(status));
    }

    template < typename Contexts >
    static void GetCapabilitiesSc(Contexts contexts,
                                  std::shared_ptr<Response> response,
                                  std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get capabilities schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetCapabilities, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<tuple<Result, PTZCapabilitiesInformation>>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, capabilities] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(capabilities));
    }

    template < typename Contexts >
    static void GetNodesSc(Contexts contexts,
                           std::shared_ptr<Response> response,
                           std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get nodes schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetNodes, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<tuple<Result, vector<PTZNode>>>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, nodes] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(nodes));
    }


    /* device */
    template < typename Contexts >
    static void CameraRebootSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request camera reboot schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(CameraReboot, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<tuple<Result, string>>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, some_message] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(ptz_result));
    }

    template < typename Contexts >
    static void GetDeviceInformationSc(Contexts contexts,
                                       std::shared_ptr<Response> response,
                                       std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get device inboost::formation schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetDeviceInformation, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<tuple<Result, DeviceInformation>>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, device_inf] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(device_inf));
    }


    /* media */
    template < typename Contexts >
    static void GetMediaProfilesSc(Contexts contexts,
                                   std::shared_ptr<Response> response,
                                   std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request get media profiles schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetMediaProfiles, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<tuple<Result, vector<Profile>>>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, profiles] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(profiles));
    }

    template < typename Contexts >
    static void GetStreamUriSc(Contexts contexts,
                               std::shared_ptr<Response> response,
                               std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace cameramanagercore::schemas::model;
        using namespace onvifwrapper;

        LOG(trace) << "Request get stream uri schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        StreamUriHelper input_data;
        try
        {
            auto [error, maybe_j] = get_from_body(request);
            if (!maybe_j)
            {
                LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % error;
                make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % error, 400);
                return;
            }

            input_data = maybe_j.value();
        }
        catch (...)
        {
            LOG(error) << "Can not find valid input parameters for this schema: Exta params error";
            make_4xx(response, "Can not find valid input parameters for this schema: Exta params error", 400);
            return;
        }
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(GetStreamUri, session_hash, input_data.stream_type,
                                                                 input_data.transport_protocol);
        };

        auto maybe_ptz_result = (*camera_pool)
            | boost::packaged_task<tuple<bool, std::optional<tuple<Result, MediaUri>>>> { action }
            | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, data] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        auto& [ptz_result, media_uri] = data.value();
        if (ptz_result.is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result));
            return;
        }

        make_200(response, nlohmann::json(media_uri));
    }

    template < typename Contexts >
    static void StartMulticastSc(Contexts contexts,
                                 std::shared_ptr<Response> response,
                                 std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request start multicast schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(StartMulticast, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }

    template < typename Contexts >
    static void StopMulticastSc(Contexts contexts,
                                std::shared_ptr<Response> response,
                                std::shared_ptr<Request> request) noexcept
    {
        using namespace cameramanagercore::cameras;
        using namespace onvifwrapper;

        LOG(trace) << "Request stop muticast schema";

        auto [signal_context, model_context, event_context,
              cameras_context, session_context, pool_context] = contexts.contexts;
        auto camera_pool = pool_context->pools[cameramanagercore::thread_pool::ThreadPoolType::Cameras];

        auto [session_hash] = get_from_header<string>(request, "session_hash").second.value();
        auto [message, maybe_input] = get_from_url<int>(ParseFromUrl(request->path_match[1]), "camera_id");
        if (!maybe_input)
        {
            LOG(error) << boost::format("Can not find valid input parameters for this schema: %1%") % message;
            make_4xx(response, boost::format("Can not find valid input parameters for this schema: %1%") % message, 400);
            return;
        }

        auto [camera_id] = maybe_input.value();
        auto maybe_camera = model_context->FindCameraById(camera_id);
        if (!maybe_camera)
        {
            LOG(error) << boost::format("Can not find camera by given id: %1%") % camera_id;
            make_5xx(response, boost::format("Can not find camera by given id: %1%") % camera_id, 500);
            return;
        }
        auto camera = maybe_camera.value();

        auto action = [&, session_hash = session_hash, cameras_context = cameras_context]
        {
            return (*cameras_context) | camera | std::make_tuple(StopMulticast, session_hash);
        };

        auto maybe_ptz_result = (*camera_pool)
                                | boost::packaged_task<tuple<bool, std::optional<Result>>> { action }
                                | cameramanagercore::thread_pool::execute;
        if (!maybe_ptz_result)
        {
            LOG(error) << "Error in request executing (task pool error)";
            make_5xx(response, "Error in request executing (task pool error)", 500);
            return;
        }

        auto [locked, ptz_result] = maybe_ptz_result.value();
        if (locked)
        {
            LOG(error) << "Error in request executing (Camera locked. Unlock first for interaction)";
            make_5xx(response, "Error in request executing (Camera locked. Unlock first for interaction)", 500);
            return;
        }
        if (ptz_result.value().is_error)
        {
            LOG(error) << "Error in ptz request (request successfully executed, but returned error)";
            make_503(response, nlohmann::json(ptz_result.value()));
            return;
        }

        make_200(response, nlohmann::json(ptz_result.value()));
    }
};

}

#endif //CAMERAMANAGERCORE_CAMERAS_HPP
