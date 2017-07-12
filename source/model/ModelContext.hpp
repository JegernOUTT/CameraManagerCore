//
// Created by svakhreev on 03.03.17.
//

#ifndef CAMERAMANAGERCORE_MODELCONTEXT_HPP
#define CAMERAMANAGERCORE_MODELCONTEXT_HPP

#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <iostream>
#include <string>
#include <algorithm>
#include <tuple>
#include <optional>
#include <string_view>
#include <type_traits>
#include <range/v3/all.hpp>
#include <boost/format.hpp>
#include <rttr/registration>

#include "./detail/ContextMisc.hpp"
#include "./Camera.hpp"
#include "./Permission.hpp"
#include "./Settings.hpp"
#include "./User.hpp"
#include "./UserCameras.hpp"
#include "./UserPermissions.hpp"
#include "./Sessions.hpp"
#include "./CameraInformation.hpp"
#include "./EventArchiveSettings.hpp"

#include "../signals/SignalContext.hpp"
#include "../utility/LoggerSettings.hpp"
#include "../utility/PasswordCryptor.hpp"

namespace cameramanagercore::model
{

using namespace ranges;
using namespace rttr;
using namespace cameramanagercore::utility;
using namespace cameramanagercore::signals;

template < typename SignalContext, typename... Types >
class ModelContext
{
public:
    using CameraPtr                 = std::shared_ptr<Camera>;
    using PermissionPtr             = std::shared_ptr<Permission>;
    using SettingsPtr               = std::shared_ptr<Settings>;
    using UserPtr                   = std::shared_ptr<User>;
    using UserCamerasPtr            = std::shared_ptr<UserCameras>;
    using UserPermissionsPtr        = std::shared_ptr<UserPermissions>;
    using SessionsPtr               = std::shared_ptr<Sessions>;
    using CameraActionPtr           = std::shared_ptr<CameraAction>;
    using EventArchiveSettingsPtr   = std::shared_ptr<EventArchiveSettings>;

    using CameraWPtr                = std::weak_ptr<Camera>;
    using PermissionWPtr            = std::weak_ptr<Permission>;
    using SettingsWPtr              = std::weak_ptr<Settings>;
    using UserWPtr                  = std::weak_ptr<User>;
    using UserCamerasWPtr           = std::weak_ptr<UserCameras>;
    using UserPermissionsWPtr       = std::weak_ptr<UserPermissions>;
    using SessionsWPtr              = std::weak_ptr<Sessions>;
    using CameraActionWPtr          = std::weak_ptr<CameraAction>;
    using EventArchiveSettingsWPtr  = std::weak_ptr<EventArchiveSettings>;

    using SignalContextPtr          = std::shared_ptr<SignalContext>;

    template < typename T >
    using SignalGeneratorT          = std::shared_ptr<signals::SignalGenerator<T>>;

    ModelContext(SignalContextPtr signal_context,
                 std::vector<CameraPtr> cameras = {},
                 std::vector<UserPtr> users = {},
                 std::vector<SettingsPtr> settings = {},
                 std::vector<PermissionPtr> permissions = {},
                 std::vector<UserCamerasPtr> usercameras = {},
                 std::vector<UserPermissionsPtr> userpermissions = {},
                 std::vector<SessionsPtr> sessions = {},
                 std::vector<CameraActionPtr> cameraaction = {},
                 std::vector<EventArchiveSettingsPtr> eventarchivesettings = {}) noexcept :
        _cameras(cameras),
        _users(users),
        _settings(settings),
        _permissions(permissions),
        _usercameras(usercameras),
        _userpermissions(userpermissions),
        _sessions(sessions),
        _cameraaction(cameraaction),
        _eventarchivesettings(eventarchivesettings),
        _signal_context(signal_context),
        _users_cameras_repr(100),
        _users_permissions_repr(100),
        _signal_generator(std::make_tuple(_signal_context->template AddSignalGenerator<Types>()...))
    {
        FillRepresentations();
    }

    ~ModelContext() noexcept
    {
        (_signal_context->RemoveSignalGenerator(std::get<SignalGeneratorT<Types>>(_signal_generator)), ...);
    }

public:
    std::optional<CameraPtr> FindCameraById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_cameras, [&id](auto&& camera) { return camera->id == id; });
        return ptr != _cameras.cend() ? *ptr : std::optional<CameraPtr>{};
    }

    std::optional<UserPtr> FindUserById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_users, [&id](auto&& user) { return user->id == id; });
        return ptr != _users.cend() ? *ptr : std::optional<UserPtr>{};
    }

    std::optional<UserPtr> FindUserByNamePassword(const User& user) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);

        auto hash = PasswordCryptor::Crypt(user.password);
        auto ptr = v3::find_if(_users,
                               [&](auto&& _user) { return _user->name == user.name
                                                          && _user->password == hash; });
        return ptr != _users.cend() ? *ptr : std::optional<UserPtr>{};
    }

    std::optional<PermissionPtr> FindPermissionById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_permissions, [&id](auto&& perm) { return perm->id == id; });
        return ptr != _permissions.cend() ? *ptr : std::optional<PermissionPtr>{};
    }

    std::optional<SettingsPtr> FindSettingsById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_settings, [&id](auto&& set) { return set->id == id; });
        return ptr != _settings.cend() ? *ptr : std::optional<SettingsPtr>{};
    }

    std::optional<std::vector<CameraWPtr>> GetCamerasByUser(UserPtr user) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = _users_cameras_repr.find(UserWPtr {user});
        return ptr != _users_cameras_repr.end() ? (*ptr).second : std::optional<std::vector<CameraWPtr>>{};
    }

    std::optional<std::vector<PermissionWPtr>> GetPermissionsByUser(UserPtr user) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = _users_permissions_repr.find(UserWPtr {user});
        return ptr != _users_permissions_repr.end() ? (*ptr).second : std::optional<std::vector<PermissionWPtr>>{};
    }

    std::optional<SessionsPtr> FindSessionByHash(std::string_view session_hash) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_sessions, [&](auto&& session) { return session->session_hash == session_hash; }) ;
        return ptr != _sessions.end() ? *ptr : std::optional<SessionsPtr>{};
    }

    std::optional<CameraActionPtr> FindCameraActionById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_cameraaction, [&id](auto&& c) { return c->id == id; });
        return ptr != _cameraaction.cend() ? *ptr : std::optional<CameraActionPtr>{};
    }

    std::optional<EventArchiveSettingsPtr> FindEventArchiveSettingsById(int id) const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        auto ptr = v3::find_if(_eventarchivesettings, [&id](auto&& c) { return c->id == id; });
        return ptr != _eventarchivesettings.cend() ? *ptr : std::optional<EventArchiveSettingsPtr>{};
    }

public:
    std::vector<CameraPtr> GetCameras() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _cameras;
    };
    std::vector<UserPtr> GetUsers() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _users;
    };
    std::vector<SettingsPtr> GetSettings() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _settings;
    };
    std::vector<PermissionPtr> GetPermissions() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _permissions;
    };
    std::vector<UserCamerasPtr> GetUserCameras() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _usercameras;
    };
    std::vector<UserPermissionsPtr> GetUserPermissions() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _userpermissions;
    };
    std::vector<SessionsPtr> GetSessions() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _sessions;
    };
    std::vector<CameraActionPtr> GetCameraAction() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _cameraaction;
    };
    std::vector<EventArchiveSettingsPtr> GetEventArchiveSettings() const noexcept
    {
        std::lock_guard<std::recursive_mutex> lg(_mu_main);
        return _eventarchivesettings;
    };

    OperationStatus Add(CameraPtr value) noexcept
    {
        return _ImplAdd(value, _cameras, "Camera");
    }
    OperationStatus Add(UserPtr value) noexcept
    {
        auto hash = PasswordCryptor::Crypt(value->password);
        value->password = hash;
        return _ImplAdd(value, _users, "User");
    }
    OperationStatus Add(SettingsPtr value) noexcept
    {
        return _ImplAdd(value, _settings, "Setting");
    }
    OperationStatus Add(PermissionPtr value) noexcept
    {
        return _ImplAdd(value, _permissions, "Permission");
    }
    OperationStatus Add(UserCamerasPtr value) noexcept
    {
        return _ImplAdd(value, _usercameras, "User-cameras link");
    }
    OperationStatus Add(UserPermissionsPtr value) noexcept
    {
        return _ImplAdd(value, _userpermissions, "User-permissions link");
    }
    OperationStatus Add(SessionsPtr value) noexcept
    {
        return _ImplAdd(value, _sessions, "Sessions");
    }
    OperationStatus Add(CameraActionPtr value) noexcept
    {
        return _ImplAdd(value, _cameraaction, "CameraAction");
    }
    OperationStatus Add(EventArchiveSettingsPtr value) noexcept
    {
        return _ImplAdd(value, _eventarchivesettings, "EventArchiveSettings");
    }

    std::tuple<OperationStatus, CameraPtr> Change(CameraPtr old_value, CameraPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _cameras, "Camera");
    }
    std::tuple<OperationStatus, UserPtr> Change(UserPtr old_value, UserPtr new_value) noexcept
    {
        auto hash = PasswordCryptor::Crypt(new_value->password);
        new_value->password = hash;
        return _ImplChange(old_value, new_value, _users, "User");
    }
    std::tuple<OperationStatus, SettingsPtr> Change(SettingsPtr old_value, SettingsPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _settings, "Settings");
    }
    std::tuple<OperationStatus, PermissionPtr> Change(PermissionPtr old_value, PermissionPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _permissions, "Permissions");
    }
    std::tuple<OperationStatus, SessionsPtr> Change(SessionsPtr old_value, SessionsPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _sessions, "Sessions");
    }
    std::tuple<OperationStatus, CameraActionPtr> Change(CameraActionPtr old_value, CameraActionPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _cameraaction, "CameraAction");
    }
    std::tuple<OperationStatus, EventArchiveSettingsPtr> Change(EventArchiveSettingsPtr old_value,
                                                                EventArchiveSettingsPtr new_value) noexcept
    {
        return _ImplChange(old_value, new_value, _eventarchivesettings, "EventArchiveSettings");
    }


    OperationStatus Remove(CameraPtr value) noexcept
    {
        auto on_success = [=] {
            _usercameras |= action::remove_if([&](const auto& uc) { return uc->camera_id == value->id; });
            _ImplRemoveFromMap(value, _users_cameras_repr);
        };

        return _ImplRemove(value, _cameras, "Camera", on_success);
    }
    OperationStatus Remove(UserPtr value) noexcept
    {
        auto on_success = [=] {
            _usercameras |= action::remove_if([&](const auto& uc) { return uc->user_id == value->id; });
            _userpermissions |= action::remove_if([&](const auto& up) { return up->user_id == value->id; });
            _ImplRemoveUserFromMap(value, _users_cameras_repr);
            _ImplRemoveUserFromMap(value, _users_permissions_repr);
        };

        return _ImplRemove(value, _users, "User", on_success);
    }
    OperationStatus Remove(SettingsPtr value) noexcept
    {
        return _ImplRemove(value, _settings, "Settings");
    }
    OperationStatus Remove(PermissionPtr value) noexcept
    {
        auto on_success = [=] {
            _userpermissions |= action::remove_if([&](const auto& up) { return up->permission_id == value->id; });
            _ImplRemoveFromMap(value, _users_permissions_repr);
        };

        return _ImplRemove(value, _permissions, "Permission", on_success);
    }
    OperationStatus Remove(UserCamerasPtr value) noexcept
    {
        return _ImplRemove(value, _usercameras, "User-camera link");
    }
    OperationStatus Remove(UserPermissionsPtr value) noexcept
    {
        return _ImplRemove(value, _userpermissions, "User-permission link");
    }
    OperationStatus Remove(SessionsPtr value) noexcept
    {
        return _ImplRemove(value, _sessions, "Sessions");
    }
    OperationStatus Remove(CameraActionPtr value) noexcept
    {
        return _ImplRemove(value, _cameraaction, "CameraAction");
    }
    OperationStatus Remove(EventArchiveSettingsPtr value) noexcept
    {
        return _ImplRemove(value, _eventarchivesettings, "EventArchiveSettings");
    }

    std::unordered_map<UserWPtr, std::vector<CameraWPtr>>     GetUserCamerasLink() const noexcept     { return _users_cameras_repr; };
    std::unordered_map<UserWPtr, std::vector<PermissionWPtr>> GetUserPermissionsLink() const noexcept { return _users_permissions_repr; };

    OperationStatus AddLink(UserPtr user, CameraPtr value) noexcept
    {
        return _ImplAddLink(user, value, _usercameras, "user-cameras link");
    }
    OperationStatus AddLink(UserPtr user, PermissionPtr value) noexcept
    {
        return _ImplAddLink(user, value, _userpermissions, "user-cameras link");
    }

    OperationStatus RemoveLink(UserPtr user, CameraPtr value) noexcept
    {
        return _ImplRemoveLink(user, value, _usercameras, "User-camera link");
    }
    OperationStatus RemoveLink(UserPtr user, PermissionPtr value) noexcept
    {
        return _ImplRemoveLink(user, value, _userpermissions, "User-permission link");
    }

private:
    template<typename T>
    OperationStatus _ImplAdd(std::shared_ptr<T> value,
                             std::vector<std::shared_ptr<T>>& arr,
                             std::string_view type_name) noexcept
    {
        std::unique_lock<std::recursive_mutex> ul(_mu_main);
        LOG(info) << boost::format("New %2% try add: %1%") % *value % type_name;

        auto status = IsSame(value, arr);
        if (status != OperationStatus::Ok)
        {
            LOG(error) << boost::format("%3% (%1%) adding error: %2%") % *value % type::get<OperationStatus>()
                    .get_enumeration()
                    .value_to_name(status)
                          % type_name;
            return status;
        }

        if constexpr (std::is_same_v<T, UserCameras> || std::is_same_v<T, UserPermissions>)
        {
            if (!_ImplFillData(value))
            {
                LOG(error) << boost::format("%2% (%1%) adding error: NotFound") % *value % type_name;
                return OperationStatus::NotFound;
            }
        }

        arr.emplace_back(value);
        ul.unlock();
        (*std::get<SignalGeneratorT<T>>(_signal_generator)).GenerateSignal(create_signal_data(DataStatus::Added, value));
        LOG(info) << boost::format("%2% (%1%) adding ok") % *value % type_name;
        return status;
    }

    template<typename T>
    std::tuple<OperationStatus, std::shared_ptr<T>> _ImplChange(std::shared_ptr<T> old_value,
                                                                std::shared_ptr<T> new_value,
                                                                std::vector<std::shared_ptr<T>>& arr,
                                                                std::string_view type_name) noexcept
    {
        std::unique_lock<std::recursive_mutex> ul(_mu_main);
        LOG(info) << boost::format("%3% try change.\nOld: %1%\nNew: %2%") % *old_value % *new_value % type_name;

        if constexpr (std::is_same_v<T, User>)
        {
            if (old_value->id != new_value->id)
            {
                LOG(error) << boost::format("%3% changing error: NotFound\nOld: %1%\nNew: %2%")
                              % *old_value
                              % *new_value
                              % type_name;
                return std::make_tuple(OperationStatus::NotFound, std::shared_ptr<T>{});
            }
        }
        else if constexpr (std::is_same_v<T, Sessions>)
        {
            if (old_value->session_hash != new_value->session_hash)
            {
                LOG(error) << boost::format("%3% changing error: NotFound\nOld: %1%\nNew: %2%")
                              % *old_value
                              % *new_value
                              % type_name;
                return std::make_tuple(OperationStatus::NotFound, std::shared_ptr<T>{});
            }
        }
        else
        {
            if (old_value->id != new_value->id)
            {
                LOG(error) << boost::format("%3% changing error: NotFound\nOld: %1%\nNew: %2%")
                              % *old_value
                              % *new_value
                              % type_name;
                return std::make_tuple(OperationStatus::NotFound, std::shared_ptr<T>{});
            }
        }


        auto [status, finded] = Exist(old_value, arr);
        if (status != OperationStatus::Ok)
        {
            LOG(error) << boost::format("%4% changing error: %3%\nOld: %1%\nNew: %2%")
                          % *old_value
                          % *new_value
                          % type::get<OperationStatus>()
                              .get_enumeration()
                              .value_to_name(status)
                          % type_name;
            return std::make_tuple(status, std::shared_ptr<T>{});
        }

        *finded = *new_value;
        ul.unlock();
        (*std::get<SignalGeneratorT<T>>(_signal_generator)).GenerateSignal(create_signal_data(DataStatus::Changed, finded));
        LOG(info) << boost::format("%3% changing ok.\nOld: %1%\nNew: %2%") % *old_value % *new_value % type_name;
        return std::make_tuple(status, finded);
    }

    template<typename T, typename ExtraFunctor = std::function<void()>>
    OperationStatus _ImplRemove(std::shared_ptr<T> value,
                                std::vector<std::shared_ptr<T>>& arr,
                                std::string_view type_name,
                                ExtraFunctor&& func = [] {}) noexcept
    {
        std::unique_lock<std::recursive_mutex> ul(_mu_main);
        LOG(info) << boost::format("%2% try remove: %1%") % *value % type_name;

        auto [status, finded] = Exist(value, arr);
        if (status != OperationStatus::Ok)
        {
            LOG(error) << boost::format("%3% (%1%) removing error: %2%")
                          % *value
                          % type::get<OperationStatus>()
                              .get_enumeration()
                              .value_to_name(status)
                          % type_name;
            return status;
        }

        if constexpr (std::is_same_v<T, UserCameras> || std::is_same_v<T, UserPermissions>)
        {
            if (!_ImplRemoveFromUserObject(value))
            {
                LOG(error) << boost::format("%2% (%1%) removing error: NotFound") % *value % type_name;
                return OperationStatus::NotFound;
            }
        }

        func();
        arr |= action::remove_if([&](const auto& up) { return *up == *value; });
        ul.unlock();

        (*std::get<SignalGeneratorT<T>>(_signal_generator)).GenerateSignal(create_signal_data(DataStatus::Removed, value));
        LOG(info) << boost::format("%2% removing ok: %1%") % *value % type_name;
        return status;
    }

    template<typename T,  typename U>
    OperationStatus _ImplAddLink(UserPtr user,
                                 std::shared_ptr<T> value,
                                 std::vector<std::shared_ptr<U>>& arr,
                                 std::string_view type_name) noexcept
    {
        std::unique_lock<std::recursive_mutex> ul(_mu_main);
        auto data = std::make_shared<U>(U {user->id, value->id});
        LOG(info) << boost::format("New %2% try add: %1%") % *data % type_name;

        auto status = IsSame(data, arr);
        if (status != OperationStatus::Ok)
        {
            LOG(error) << boost::format("%3% (%1%) adding error: %2%") % *data % type::get<OperationStatus>()
                    .get_enumeration()
                    .value_to_name(status)
                    % type_name;
            return status;
        }

        if (!_ImplFillData(data))
        {
            LOG(error) << boost::format("%2% (%1%) adding error: NotFound") % *data % type_name;
            return OperationStatus::NotFound;
        }

        arr.emplace_back(data);
        ul.unlock();

        (*std::get<SignalGeneratorT<U>>(_signal_generator)).GenerateSignal(create_signal_data(DataStatus::Added, data));
        LOG(info) << boost::format("%2% (%1%) adding ok") % *data % type_name;
        return status;
    }

    template<typename T, typename U>
    OperationStatus _ImplRemoveLink(UserPtr user,
                                    std::shared_ptr<T> value,
                                    std::vector<std::shared_ptr<U>>& arr,
                                    std::string_view type_name)
    {
        std::unique_lock<std::recursive_mutex> ul(_mu_main);
        auto data = std::make_shared<U>(U {user->id, value->id});
        LOG(info) << boost::format("%2% try remove: %1%") % *data % type_name;

        auto [status, finded] = Exist(data, arr);
        if (status != OperationStatus::Ok)
        {
            LOG(error) << boost::format("%3% (%1%) removing error: %2%")
                          % *data
                          % type::get<OperationStatus>()
                              .get_enumeration()
                              .value_to_name(status)
                          % type_name;
            return status;
        }

        if (!_ImplRemoveFromUserObject(data))
        {
            LOG(error) << boost::format("User-permission link (%1%) removing error: NotFound") % *data;
            return OperationStatus::NotFound;
        }

        if constexpr (std::is_same_v<T, UserCameras>)
        {
            arr |= action::remove_if([&](const auto& p) { return p->user_id == user->id && p->camera_id == value->id; });
        }
        if constexpr (std::is_same_v<T, UserPermissions>)
        {
            arr |= action::remove_if([&](const auto& p) { return p->user_id == user->id && p->permission_id == value->id; });
        }

        ul.unlock();

        (*std::get<SignalGeneratorT<U>>(_signal_generator)).GenerateSignal(create_signal_data(DataStatus::Removed, data));
        LOG(info) << boost::format("%2% removing ok: %1%") % *data % type_name;
        return status;
    }

private:
    void FillRepresentations()
    {
        _users_cameras_repr.clear();
        _users_permissions_repr.clear();

        for (auto&& user_camera: _usercameras)
        {
            _ImplFillData(user_camera);
        }

        for (auto&& user_perm: _userpermissions)
        {
            _ImplFillData(user_perm);
        }
    }
    bool _ImplFillData(const UserCamerasPtr& user_camera) noexcept
    {
        auto user = FindUserById(user_camera->user_id);
        auto camera = FindCameraById(user_camera->camera_id);

        if (!user || !camera) return false;
        auto find_by_user = _users_cameras_repr.find(UserWPtr {user.value()});

        if (find_by_user != _users_cameras_repr.end())
        {
            (*find_by_user).second.emplace_back(CameraWPtr {camera.value()});
        }
        else
        {
            _users_cameras_repr.emplace(UserWPtr {user.value()},
                                        std::vector<CameraWPtr> {CameraWPtr {camera.value()}});
        }
        return true;
    }
    bool _ImplFillData(const UserPermissionsPtr& user_perm) noexcept
    {
        auto user = FindUserById(user_perm->user_id);
        auto perm = FindPermissionById(user_perm->permission_id);

        if (!user || !perm) return false;
        auto find_by_user = _users_permissions_repr.find(UserWPtr {user.value()});

        if (find_by_user != _users_permissions_repr.end())
        {
            (*find_by_user).second.emplace_back(PermissionWPtr {perm.value()});
        }
        else
        {
            _users_permissions_repr.emplace(UserWPtr {user.value()},
                                            std::vector<PermissionWPtr> {PermissionWPtr {perm.value()}});
        }
        return true;
    }
    bool _ImplRemoveFromUserObject(const UserCamerasPtr& user_camera) noexcept
    {
        auto user = FindUserById(user_camera->user_id);
        auto camera = FindCameraById(user_camera->camera_id);

        if (!user || !camera) return false;
        auto find_by_user = _users_cameras_repr.find(UserWPtr {user.value()});
        if (find_by_user == _users_cameras_repr.end()) return false;

        find_by_user->second |= action::remove_if([&](const auto& finded) { return camera.value() == finded.lock(); });
        return true;
    }
    bool _ImplRemoveFromUserObject(const UserPermissionsPtr& user_perm) noexcept
    {
        auto user = FindUserById(user_perm->user_id);
        auto perm = FindPermissionById(user_perm->permission_id);

        if (!user || !perm) return false;
        auto find_by_user = _users_permissions_repr.find(UserWPtr {user.value()});
        if (find_by_user == _users_permissions_repr.end()) return false;

        find_by_user->second |= action::remove_if([&](const auto& finded) { return perm.value() == finded.lock(); });
        return true;
    }
    template<typename T>
    bool _ImplRemoveUserFromMap(const UserPtr& user, std::unordered_map<UserWPtr, std::vector<std::weak_ptr<T>>>& m) noexcept
    {
        auto finded_user = FindUserById(user->id);
        if (!finded_user) return false;

        auto count = m.erase(UserWPtr{finded_user.value()});
        return count >= 1 ? true : false;
    }
    template<typename T>
    bool _ImplRemoveFromMap(const std::shared_ptr<T>& value, std::unordered_map<UserWPtr, std::vector<std::weak_ptr<T>>>& m) noexcept;
    bool _ImplRemoveFromMap(const CameraPtr& camera, std::unordered_map<UserWPtr, std::vector<CameraWPtr>>& m) noexcept
    {
        auto finded_camera = FindCameraById(camera->id);
        if (!finded_camera) return false;

        for (auto& p: m)
            p.second |= action::remove_if([&] (auto& v) { return v == CameraWPtr{finded_camera.value()}; });

        return true;
    }
    bool _ImplRemoveFromMap(const PermissionPtr& perm, std::unordered_map<UserWPtr, std::vector<PermissionWPtr>>& m) noexcept
    {
        auto finded_perm = FindPermissionById(perm->id);
        if (!finded_perm) return false;

        for (auto&& [user, arr]: m)
            arr |= action::remove_if([&] (auto& v) { return v == PermissionWPtr{finded_perm.value()}; });

        return true;
    }

    template<typename T>
    OperationStatus IsSame(const std::shared_ptr<T>& value, const std::vector<std::shared_ptr<T>>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        if (by_value != arr.cend()) return OperationStatus::SameBody;
        auto by_id = v3::find_if(arr, [&] (const auto& v) { return v->id == value->id; });
        if (by_id != arr.cend()) return OperationStatus::SameId;
        return OperationStatus::Ok;
    }
    OperationStatus IsSame(const UserPtr& value, const std::vector<UserPtr>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        if (by_value != arr.cend()) return OperationStatus::SameBody;
        auto by_id = v3::find_if(arr, [&] (const auto& v) { return v->id == value->id; });
        if (by_id != arr.cend()) return OperationStatus::SameId;
        return OperationStatus::Ok;
    }
    OperationStatus IsSame(const SessionsPtr& value, const std::vector<SessionsPtr>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        if (by_value != arr.cend()) return OperationStatus::SameBody;
        auto by_id = v3::find_if(arr, [&] (const auto& v) { return v->session_hash == value->session_hash; });
        if (by_id != arr.cend()) return OperationStatus::SameId;
        return OperationStatus::Ok;
    }
    OperationStatus IsSame(const UserCamerasPtr& value, const std::vector<UserCamerasPtr>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        if (by_value != arr.cend()) return OperationStatus::SameBody;
        return OperationStatus::Ok;
    }
    OperationStatus IsSame(const UserPermissionsPtr& value, const std::vector<UserPermissionsPtr>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        if (by_value != arr.cend()) return OperationStatus::SameBody;
        return OperationStatus::Ok;
    }
    template<typename T>
    std::tuple<OperationStatus, std::shared_ptr<T>> Exist(const std::shared_ptr<T>& value, const std::vector<std::shared_ptr<T>>& arr) const noexcept
    {
        auto by_value = v3::find_if(arr, [&] (const auto& v) { return *v == *value; });
        return by_value != arr.cend() ? std::make_tuple(OperationStatus::Ok, *by_value)
                                      : std::make_tuple(OperationStatus::NotFound, std::shared_ptr<T>{});
    }

private:
    std::vector<CameraPtr> _cameras;
    std::vector<UserPtr> _users;
    std::vector<SettingsPtr> _settings;
    std::vector<PermissionPtr> _permissions;
    std::vector<UserCamerasPtr> _usercameras;
    std::vector<UserPermissionsPtr> _userpermissions;
    std::vector<SessionsPtr> _sessions;
    std::vector<CameraActionPtr> _cameraaction;
    std::vector<EventArchiveSettingsPtr> _eventarchivesettings;
    SignalContextPtr _signal_context;

    std::unordered_map<UserWPtr, std::vector<CameraWPtr>> _users_cameras_repr;
    std::unordered_map<UserWPtr, std::vector<PermissionWPtr>> _users_permissions_repr;

    std::tuple<SignalGeneratorT<Types>...> _signal_generator;

    mutable std::recursive_mutex _mu_main;
};

using Model = ModelContext< SignalsContext,
                            Camera,
                            Permission,
                            Settings,
                            User,
                            UserCameras,
                            UserPermissions,
                            Sessions,
                            CameraAction,
                            EventArchiveSettings >;

}

#endif //CAMERAMANAGERCORE_MODELCONTEXT_HPP
