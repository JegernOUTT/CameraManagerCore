//
// Created by svakhreev on 03.03.17.
//

#include <string>
#include <functional>
#include <memory>
#include <future>
#include <random>
#include <type_traits>
#include <sstream>
#include <map>

#include <bandit/bandit.h>
#include <rttr/registration>
#include <range/v3/all.hpp>

#include "../source/signals/SignalContext.hpp"
#include "../source/utility/RandomGenerator.hpp"
#include "../source/utility/PasswordCryptor.hpp"
#include "../source/model/ModelContext.hpp"

using namespace bandit;
using namespace ranges;
using namespace rttr;
using namespace std::chrono;
using namespace cameramanagercore::model;
using namespace cameramanagercore::signals;
using namespace cameramanagercore::utility;

using std::move;
using std::make_unique;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::map;
using std::is_same_v;

template<typename T>
struct Generator
{
    T operator()()
    {
        T value;
        auto type = type::get<T>();
        for (auto& p: type.get_properties())
        {
            if (p.get_type().is_arithmetic())
            {
                p.set_value(value, generate_num());
            }
            else if (p.get_type() == type::get<string>())
            {
                p.set_value(value, generate_str());
            }
        }
        return value;
    }

    T operator()(const T& value)
    {
        auto type = type::get<T>();
        for (auto& p: type.get_properties())
        {
            if (p.get_type().is_arithmetic())
            {
                p.set_value(value, generate_num());
            }
            else if (p.get_type() == type::get<string>())
            {
                p.set_value(value, generate_str());
            }
        }
        return value;
    }

private:
    int generate_num()
    {
        return RandomGenerator::RandomInt(0, 10'000);
    }

    string generate_str()
    {
        return RandomGenerator::RandomString();
    }
};

template<typename T>
struct ModelTest
{
    ModelTest() : model(make_shared<SignalsContext>())
    {
        Add();
        Change();
        Remove();
    }

private:
    Model model;
    vector<T> all_data;

    void Add()
    {
        int counter = 0;
        vector<T> range = view::generate_n(Generator<T>{}, 1'000);
        for_each(range, [&](auto& item) { item.id = ++counter;
                                          auto status = model.Add(make_shared<T>(item));
                                          AssertThat(status, Is().EqualTo(OperationStatus::Ok)); });

        counter = 0;
        for_each(range, [&](auto& item) { item.id = ++counter;
                                          auto status = model.Add(make_shared<T>(item));
                                          AssertThat(status, Is().EqualTo(OperationStatus::SameBody)); });

        counter = 0;
        vector<T> range_ = view::generate_n(Generator<T>{}, 1'000);
        for_each(range_, [&](auto& item) { item.id = ++counter;
                                           auto status = model.Add(make_shared<T>(item));
                                           AssertThat(status, Is().EqualTo(OperationStatus::SameId)); });
        for_each(range_, [&](auto& item) { item.id = ++counter;
                                           auto status = model.Add(make_shared<T>(item));
                                           AssertThat(status, Is().EqualTo(OperationStatus::Ok)); });
        all_data = view::concat(range, range_);


        if constexpr (is_same_v<T, Camera>)
        {
            auto raw = model.GetCameras();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            AssertThat(items, Is().EqualToContainer(all_data));
        }
        else if constexpr (is_same_v<T, User>)
        {
            auto raw = model.GetUsers();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            vector<T> all_data_ = all_data | view::transform([](auto u) { auto hash = PasswordCryptor::Crypt(u.password);
                                                                          u.password = hash;
                                                                          return u; });
            AssertThat(items, Is().EqualToContainer(all_data_));
        }
        else if constexpr (is_same_v<T, Settings>)
        {
            auto raw = model.GetSettings();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            AssertThat(items, Is().EqualToContainer(all_data));
        }
        else if constexpr (is_same_v<T, Permission>)
        {
            auto raw = model.GetPermissions();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            AssertThat(items, Is().EqualToContainer(all_data));
        }
        else if constexpr (is_same_v<T, UserCameras>)
        {
            auto raw = model.GetUserCameras();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            AssertThat(items, Is().EqualToContainer(all_data));
        }
        else if constexpr (is_same_v<T, UserPermissions>)
        {
            auto raw = model.GetUserPermissions();
            vector<T> items = raw | view::transform([](auto p) {return *p;});
            AssertThat(items, Is().EqualToContainer(all_data));
        }
    };

    void Change()
    {
        vector<T> changed_data;
        changed_data = view::generate_n(Generator<T>{}, all_data.size());
        for_each(view::zip(all_data     | view::transform([] (auto item) { return make_shared<T>(item); }),
                           changed_data | view::transform([] (auto item) { return make_shared<T>(item); })),
                 [&](auto t) { get<1>(t)->id = get<0>(t)->id;
                               auto [status, data] = model.Change(get<0>(t), get<1>(t));
                               AssertThat(status, Is().EqualTo(OperationStatus::Ok));
                               AssertThat(*data,  Is().EqualTo(*get<1>(t))); });

        for_each(view::zip(all_data     | view::transform([] (auto item) { return make_shared<T>(item); }),
                           changed_data | view::transform([] (auto item) { item.id++; return make_shared<T>(item); })),
                           [&](auto t) { auto [status, data] = model.Change(get<0>(t), get<1>(t));
                                         AssertThat(status, Is().EqualTo(OperationStatus::NotFound));
                                         AssertThat(data,   Is().EqualTo(shared_ptr<T>())); });

        for_each(view::zip(all_data     | view::transform([] (auto item) { return make_shared<T>(item); }),
                           changed_data | view::transform([] (auto item) { return make_shared<T>(item); })),
                           [&](auto t) { auto [status, data] = model.Change(get<0>(t), get<1>(t));
                                         AssertThat(status, Is().EqualTo(OperationStatus::NotFound));
                                         AssertThat(data,   Is().EqualTo(shared_ptr<T>())); });

        if constexpr (is_same_v<T, Camera>)
        {
            auto raw = model.GetCameras();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
        else if constexpr (is_same_v<T, User>)
        {
            auto raw = model.GetUsers();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
        else if constexpr (is_same_v<T, Settings>)
        {
            auto raw = model.GetSettings();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
        else if constexpr (is_same_v<T, Permission>)
        {
            auto raw = model.GetPermissions();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
        else if constexpr (is_same_v<T, UserCameras>)
        {
            auto raw = model.GetUserCameras();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
        else if constexpr (is_same_v<T, UserPermissions>)
        {
            auto raw = model.GetUserPermissions();
            all_data = raw | view::transform([](auto p) {return *p;});
        }
    }

    void Remove()
    {
        auto half_size = all_data.size() / 2;

            for_each(all_data | view::take(half_size) | view::transform([] (auto item) { return make_shared<T>(item); }),
                     [&] (auto item) { auto status = model.Remove(item);
                                       AssertThat(status, Is().EqualTo(OperationStatus::Ok)); });

            for_each(all_data | view::take(half_size) | view::transform([] (auto item) { return make_shared<T>(item); }),
                     [&] (auto item) { auto status = model.Remove(item);
                                       AssertThat(status, Is().EqualTo(OperationStatus::NotFound)); });

        if constexpr (is_same_v<T, Camera>)
        {
            auto raw = model.GetCameras();
            vector<T> in_data =   raw      | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse |  view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
        else if constexpr (is_same_v<T, User>)
        {
            auto raw = model.GetUsers();
            vector<T> in_data =   raw       | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse | view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
        else if constexpr (is_same_v<T, Settings>)
        {
            auto raw = model.GetSettings();
            vector<T> in_data =   raw      | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse | view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
        else if constexpr (is_same_v<T, Permission>)
        {
            auto raw = model.GetPermissions();
            vector<T> in_data =   raw      | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse | view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
        else if constexpr (is_same_v<T, UserCameras>)
        {
            auto raw = model.GetUserCameras();
            vector<T> in_data =   raw      | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse | view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
        else if constexpr (is_same_v<T, UserPermissions>)
        {
            auto raw = model.GetUserPermissions();
            vector<T> in_data =   raw      | view::transform([](auto p) {return *p;});
            vector<T> half_data = all_data | view::reverse | view::take(half_size) | view::reverse;
            AssertThat(in_data, Is().EqualToContainer(half_data));
        }
    }
};

go_bandit([] {
    describe("Model context tests", [] {
        shared_ptr<SignalsContext> signal_context;
        shared_ptr<Camera> camera1, camera2, camera3;
        shared_ptr<User> user1, user2, user3;
        shared_ptr<UserCameras> user_cameras1, user_cameras2, user_cameras3, user_cameras4;
        shared_ptr<UserPermissions> user_perms1, user_perms2, user_perms3, user_perms4;
        shared_ptr<Permission> perm1, perm2, perm3;

        bandit::before_each([&] {
            signal_context = make_shared<SignalsContext>();

            camera1 = make_shared<Camera>(Camera{ "Mike",   "http://google.com/fgdkbadfnaERASG",
                                                  "admin", "admin", CameraInteraction::Onvif, 21, "", 1 });
            camera2 = make_shared<Camera>(Camera{ "Sergey", "http://google.com/asg34gbsdfhaeyg",
                                                  "mikrs", "as542", CameraInteraction::Onvif, 26, "", 2 });
            camera3 = make_shared<Camera>(Camera{ "Alex",   "http://google.com/asdasfq2rdsdh23",
                                                  "kalga", "2631s", CameraInteraction::Onvif, 15, "", 3 });

            user1 = make_shared<User>(User{ "Mike",   "12345", "", 1 });
            user2 = make_shared<User>(User{ "Sergey", "65721", "", 2 });
            user3 = make_shared<User>(User{ "Alex",   "65654", "", 3 });

            user_cameras1 = make_shared<UserCameras>(UserCameras {1, 2});
            user_cameras2 = make_shared<UserCameras>(UserCameras {1, 3});
            user_cameras3 = make_shared<UserCameras>(UserCameras {2, 1});
            user_cameras4 = make_shared<UserCameras>(UserCameras {3, 3});

            user_perms1 = make_shared<UserPermissions>(UserPermissions {1, 1});
            user_perms2 = make_shared<UserPermissions>(UserPermissions {2, 2});
            user_perms3 = make_shared<UserPermissions>(UserPermissions {3, 1});
            user_perms4 = make_shared<UserPermissions>(UserPermissions {3, 3});

            perm1 = make_shared<Permission>(Permission{ "Cameras", "/cameras", 1 });
            perm2 = make_shared<Permission>(Permission{ "Ptz control", "/./ptz", 2 });
            perm3 = make_shared<Permission>(Permission{ "Test", "/./test/a124/^$&@$)", 3 });
        });

        it("Automated test", [&] {
            ModelTest<Camera>     test1;
            ModelTest<User>       test2;
            ModelTest<Permission> test3;
            ModelTest<Settings>   test4;
        });

        it("Get test", [&] {
            vector<Model::CameraPtr> c {camera1, camera2, camera3};
            vector<Model::UserPtr> u {user1, user2, user3};
            vector<Model::SettingsPtr> s;
            vector<Model::PermissionPtr> p {perm1, perm2, perm3};
            vector<Model::UserCamerasPtr> uc {user_cameras1, user_cameras2, user_cameras3, user_cameras4};
            vector<Model::UserPermissionsPtr> up {user_perms1, user_perms2, user_perms3, user_perms4};

            Model model { signal_context, c, u, s, p, uc, up };
            AssertThat(model.GetCameras(), Is().EqualToContainer(c));
            AssertThat(model.GetUsers(), Is().EqualToContainer(u));
            AssertThat(model.GetSettings(), Is().EqualToContainer(s));
            AssertThat(model.GetPermissions(), Is().EqualToContainer(p));
            AssertThat(model.GetUserCameras(), Is().EqualToContainer(uc));
            AssertThat(model.GetUserPermissions(), Is().EqualToContainer(up));
        });

        it("Conditional get test", [&] {
            vector<Model::CameraPtr> c {camera1, camera2, camera3};
            vector<Model::UserPtr> u {user1, user2, user3};
            vector<Model::SettingsPtr> s;
            vector<Model::PermissionPtr> p {perm1, perm2, perm3};
            vector<Model::UserCamerasPtr> uc {user_cameras1, user_cameras2, user_cameras3, user_cameras4};
            vector<Model::UserPermissionsPtr> up {user_perms1, user_perms2, user_perms3, user_perms4};

            Model model { signal_context, c, u, s, p, uc, up };

            auto maybe_cameras = model.GetCamerasByUser(user1);
            AssertThat(static_cast<bool>(maybe_cameras), Is().True());
            AssertThat(maybe_cameras.value().size(), Is().EqualTo(2));
            AssertThat(maybe_cameras.value()[0], Is().EqualTo(Model::CameraWPtr{camera2})
                .Or().EqualTo(Model::CameraWPtr{camera3}));
            AssertThat(maybe_cameras.value()[1], Is().EqualTo(Model::CameraWPtr{camera2})
                .Or().EqualTo(Model::CameraWPtr{camera3}));

            maybe_cameras = model.GetCamerasByUser(user2);
            AssertThat(static_cast<bool>(maybe_cameras), Is().True());
            AssertThat(maybe_cameras.value().size(), Is().EqualTo(1));
            AssertThat(maybe_cameras.value()[0], Is().EqualTo(Model::CameraWPtr{camera1}));

            maybe_cameras = model.GetCamerasByUser(user3);
            AssertThat(static_cast<bool>(maybe_cameras), Is().True());
            AssertThat(maybe_cameras.value().size(), Is().EqualTo(1));
            AssertThat(maybe_cameras.value()[0], Is().EqualTo(Model::CameraWPtr{camera3}));

            auto maybe_perms = model.GetPermissionsByUser(user1);
            AssertThat(static_cast<bool>(maybe_perms), Is().True());
            AssertThat(maybe_perms.value().size(), Is().EqualTo(1));
            AssertThat(maybe_perms.value()[0], Is().EqualTo(Model::PermissionWPtr{perm1}));

            maybe_perms = model.GetPermissionsByUser(user2);
            AssertThat(static_cast<bool>(maybe_perms), Is().True());
            AssertThat(maybe_perms.value().size(), Is().EqualTo(1));
            AssertThat(maybe_perms.value()[0], Is().EqualTo(Model::PermissionWPtr{perm2}));

            maybe_perms = model.GetPermissionsByUser(user3);
            AssertThat(static_cast<bool>(maybe_perms), Is().True());
            AssertThat(maybe_perms.value().size(), Is().EqualTo(2));
            AssertThat(maybe_perms.value()[0], Is().EqualTo(Model::PermissionWPtr{perm1})
                .Or().EqualTo(Model::PermissionWPtr{perm3}));
            AssertThat(maybe_perms.value()[1], Is().EqualTo(Model::PermissionWPtr{perm1})
                .Or().EqualTo(Model::PermissionWPtr{perm3}));
        });

        it("Add test", [&] {
            vector<Model::CameraPtr> c {camera1, camera2, camera3};
            vector<Model::UserPtr> u {user1, user2, user3};
            vector<Model::SettingsPtr> s;
            vector<Model::PermissionPtr> p {perm1, perm2, perm3};
            vector<Model::UserCamerasPtr> uc {user_cameras1, user_cameras2, user_cameras3, user_cameras4};
            vector<Model::UserPermissionsPtr> up {user_perms1, user_perms2, user_perms3, user_perms4};

            Model model { signal_context, c, u, s, p, uc, up };

            auto status = model.Add(make_shared<Camera>(Camera{ "Cam 1",
                                                                "http://google.com/fgdkbadfnaERASG",
                                                                "admin",
                                                                "admin",
                                                                CameraInteraction::Onvif, 100, "", 4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            AssertThat(model.GetCameras().size(),  Is().EqualTo(4));

            status = model.Add(make_shared<User>(User{ "Sasha",
                                                       "12345",
                                                       "",
                                                       4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            AssertThat(model.GetCameras().size(),  Is().EqualTo(4));

            status = model.Add(make_shared<Camera>(Camera{ "Cam 1",
                                                           "http://google.com/fgdkbadfnaERASG",
                                                           "admin",
                                                           "admin",
                                                           CameraInteraction::Onvif, 100, "", 4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::SameBody));
            AssertThat(model.GetCameras().size(),  Is().EqualTo(4));

            status = model.Add(make_shared<Camera>(Camera{ "Camera 2",
                                                           "http://g123oogle.com/fgdkbadfnaERASG",
                                                           "admin1",
                                                           "admin1",
                                                           CameraInteraction::Onvif, 100, "", 4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::SameId));
            AssertThat(model.GetCameras().size(), Is().EqualTo(4));

            status = model.Add(make_shared<UserCameras>(UserCameras{ 2, 4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Add(make_shared<UserPermissions>(UserPermissions{ 2, 1 }));
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));

            status = model.Add(make_shared<UserCameras>(UserCameras{ 2, 4 }));
            AssertThat(status, Is().EqualTo(OperationStatus::SameBody));
            status = model.Add(make_shared<UserPermissions>(UserPermissions{ 2, 1 }));
            AssertThat(status, Is().EqualTo(OperationStatus::SameBody));

            status = model.Add(make_shared<UserCameras>(UserCameras{ 33, 2 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));
            status = model.Add(make_shared<UserCameras>(UserCameras{ 2, 33 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            status = model.Add(make_shared<UserPermissions>(UserPermissions{ 33, 2 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));
            status = model.Add(make_shared<UserPermissions>(UserPermissions{ 2, 33 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            status = model.AddLink(user3, camera1);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.AddLink(user3, camera2);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.AddLink(user3, camera3);
            AssertThat(status, Is().EqualTo(OperationStatus::SameBody));
            status = model.AddLink(user3, make_shared<Camera>(Camera{ "Cam11era 2",
                                                                      "http://g123oogle.com/fgdkbadfnaERASG",
                                                                      "admin1",
                                                                      "ad3min1",
                                                                      CameraInteraction::Onvif, 100, "", 4123 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            status = model.AddLink(user1, perm1);
            AssertThat(status, Is().EqualTo(OperationStatus::SameBody));
            status = model.AddLink(user1, perm2);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.AddLink(user1, perm3);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.AddLink(user1, make_shared<Permission>(Permission{ "Cameras", "/camerasa", 111 }));
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            auto comparator = [](auto& weak_lhs, auto& weak_rhs) { return weak_lhs.lock()->id < weak_rhs.lock()->id; };

            auto in_sorted_user_cameras = model.GetCamerasByUser(user3).value() | action::sort(comparator);
            auto exist_sorted_user_cameras = vector<Model::CameraWPtr>{{Model::CameraWPtr{camera1},
                                                                        Model::CameraWPtr{camera2},
                                                                        Model::CameraWPtr{camera3}}} | action::sort(comparator);
            AssertThat(in_sorted_user_cameras, Is().EqualToContainer(exist_sorted_user_cameras));

            auto in_sorted_permissions = model.GetPermissionsByUser(user1).value() | action::sort(comparator);
            auto exist_sorted_permissions = vector<Model::PermissionWPtr>{{Model::PermissionWPtr{perm1},
                                                                           Model::PermissionWPtr{perm2},
                                                                           Model::PermissionWPtr{perm3}}} | action::sort(comparator);
            AssertThat(in_sorted_permissions, Is().EqualToContainer(exist_sorted_permissions));
        });
        it("Change test", [&] {
            vector<Model::CameraPtr> c {camera1, camera2, camera3};
            vector<Model::UserPtr> u {user1, user2, user3};
            vector<Model::SettingsPtr> s;
            vector<Model::PermissionPtr> p {perm1, perm2, perm3};
            vector<Model::UserCamerasPtr> uc {user_cameras1, user_cameras2, user_cameras3, user_cameras4};
            vector<Model::UserPermissionsPtr> up {user_perms1, user_perms2, user_perms3, user_perms4};

            Model model { signal_context, c, u, s, p, uc, up };

            auto user_copy = make_shared<User>(*user1);
            user_copy->name = "Dima";
            auto [status, changed] = model.Change(user1, user_copy);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            AssertThat(*changed, Is().EqualTo(*user_copy));

            user_copy = make_shared<User>(*user1);
            user_copy->id = 10;
            auto [status1, changed1] = model.Change(changed, user_copy);
            AssertThat(status1, Is().EqualTo(OperationStatus::NotFound));

            auto users = model.GetUsers();
            AssertThat(users.size(), Is().EqualTo(3));
            auto new_user_1 = make_shared<User>(*users[0]);
            auto new_user_2 = make_shared<User>(*users[1]);
            auto new_user_3 = make_shared<User>(*users[2]);
            new_user_1->name = "Sonn";
            new_user_1->password = "123456789";
            new_user_2->name = "Sonn";
            new_user_2->password = "123456789";
            new_user_3->name = "Hor";
            new_user_3->password = "999";

            model.Change(users[0], new_user_1);
            model.Change(users[1], new_user_2);
            model.Change(users[2], new_user_3);
            users = model.GetUsers();
            vector<User> pod_users {*users[0], *users[1], *users[2]};
            AssertThat(pod_users, Is().EqualToContainer(vector<User> {*new_user_1, *new_user_2, *new_user_3}));
        });
        it("Delete test", [&] {
            vector<Model::CameraPtr> c {camera1, camera2, camera3};
            vector<Model::UserPtr> u {user1, user2, user3};
            vector<Model::SettingsPtr> s;
            vector<Model::PermissionPtr> p {perm1, perm2, perm3};
            vector<Model::UserCamerasPtr> uc {user_cameras1, user_cameras2, user_cameras3, user_cameras4};
            vector<Model::UserPermissionsPtr> up {user_perms1, user_perms2, user_perms3, user_perms4};

            Model model { signal_context, c, u, s, p, uc, up };
            auto status = model.Remove(camera1);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(camera2);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(camera3);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(camera1);
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            auto camera_links = model.GetUserCamerasLink();
            AssertThat(camera_links.find(Model::UserWPtr{user1})->second.size(), Is().EqualTo(0));
            AssertThat(camera_links.find(Model::UserWPtr{user2})->second.size(), Is().EqualTo(0));
            AssertThat(camera_links.find(Model::UserWPtr{user3})->second.size(), Is().EqualTo(0));

            status = model.Remove(user1);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(user2);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(user3);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(user1);
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));
            camera_links = model.GetUserCamerasLink();
            auto perm_links = model.GetUserPermissionsLink();
            AssertThat(camera_links.size(), Is().EqualTo(0));
            AssertThat(perm_links.size(), Is().EqualTo(0));

            status = model.Remove(perm1);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(perm2);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(perm3);
            AssertThat(status, Is().EqualTo(OperationStatus::Ok));
            status = model.Remove(perm3);
            AssertThat(status, Is().EqualTo(OperationStatus::NotFound));

            AssertThat(model.GetCameras().size(), Is().EqualTo(0));
            AssertThat(model.GetUsers().size(), Is().EqualTo(0));
            AssertThat(model.GetPermissions().size(), Is().EqualTo(0));
            AssertThat(model.GetUserPermissions().size(), Is().EqualTo(0));
            AssertThat(model.GetUserCameras().size(), Is().EqualTo(0));
        });
    });
});