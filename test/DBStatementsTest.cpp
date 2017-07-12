//
// Created by svakhreev on 29.12.16.
//

#include <memory>
#include <tuple>
#include <iostream>

#include <bandit/bandit.h>
#include <sqlite_modern_cpp.h>

#include "../source/database/adapters/SqliteAdapter.hpp"
#include "../source/database/statements/AllStatements.hpp"

using namespace bandit;
using namespace cameramanagercore::database::adapters;
using namespace cameramanagercore::database::statements;
using namespace cameramanagercore::model;

using std::shared_ptr;

using std::make_shared;
using std::tie;

using AdapterPtr = shared_ptr<Adapter<sqlite::database>>;
using UserPtr = shared_ptr<User>;
using CameraPtr = shared_ptr<Camera>;
using EventPtr = shared_ptr<Event>;
using PermissionPtr = shared_ptr<Permission>;
using SettingsPtr = shared_ptr<Settings>;
using UserCamerasPtr = shared_ptr<UserCameras>;
using UserPermissionsPtr = shared_ptr<UserPermissions>;
using SessionsPtr = shared_ptr<Sessions>;
using CameraActionPtr = shared_ptr<CameraAction>;

go_bandit([]{
    describe("Sqlite user statement test", [] {
        auto user1 = make_shared<User>(User{ "Mike", "12345" });
        auto user2 = make_shared<User>(User{ "Sergey", "65721" });
        auto user3 = make_shared<User>(User{ "Alex",   "65654" });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserPtr>(adapter)
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};
            Add(adapter, { user1, user2, user3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};
        });

        it("Get test", [&] {
            auto [status, users] = Get<UserPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(users.size(), Is().EqualTo(3));
            AssertThat(*users[0], Is().EqualTo(*user1));
            AssertThat(*users[1], Is().EqualTo(*user2));
            AssertThat(*users[2], Is().EqualTo(*user3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserPtr>(adapter);

            Add(adapter, { user1, user2, user3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, users] = Get<UserPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};
            AssertThat(users.size(), Is().EqualTo(3));
            AssertThat(*users[0], Is().EqualTo(*user1));
            AssertThat(*users[1], Is().EqualTo(*user2));
            AssertThat(*users[2], Is().EqualTo(*user3));
        });

        it("Change test", [&] {
            user1->name = "Some other name";
            user1->password = "as65f6a4svsw56r7qrwfg456xcbn4d6";

            user3->password = "gasdg8as7gnj124mn xm,chsdjkg";
            user3->name = "Victor";

            Change(adapter, { user1, user2, user3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, users] = Get<UserPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(users.size(), Is().EqualTo(3));
            AssertThat(*users[0], Is().EqualTo(*user1));
            AssertThat(*users[1], Is().EqualTo(*user2));
            AssertThat(*users[2], Is().EqualTo(*user3));
        });

        it("Remove test", [&] {
            Remove(adapter, { user3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, users] = Get<UserPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(users.size(), Is().EqualTo(2));
            AssertThat(*users[0], Is().EqualTo(*user1));
            AssertThat(*users[1], Is().EqualTo(*user2));
        });
    });

    describe("Sqlite camera statement test", [] {
        auto camera1 = make_shared<Camera>(Camera{ "Mike",   "http://google.com/fgdkbadfnaERASG",
                                                   "admin", "admin", CameraInteraction::Onvif });
        auto camera2 = make_shared<Camera>(Camera{ "Sergey", "http://google.com/asg34gbsdfhaeyg",
                                                   "mikrs", "as542", CameraInteraction::Onvif });
        auto camera3 = make_shared<Camera>(Camera{ "Alex",   "http://google.com/asdasfq2rdsdh23",
                                                   "kalga", "2631s", CameraInteraction::Axis });
        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<CameraPtr>(adapter);
            Add(adapter, { camera1, camera2, camera3 });
        });

        it("Get test", [&] {
            auto [status, cameras] = Get<CameraPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(cameras.size(), Is().EqualTo(3));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
            AssertThat(*cameras[1], Is().EqualTo(*camera2));
            AssertThat(*cameras[2], Is().EqualTo(*camera3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<CameraPtr>(adapter);

            Add(adapter, { camera1, camera2, camera3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, cameras] = Get<CameraPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(cameras.size(), Is().EqualTo(3));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
            AssertThat(*cameras[1], Is().EqualTo(*camera2));
            AssertThat(*cameras[2], Is().EqualTo(*camera3));
        });

        it("Change test", [&] {
            camera1->name = "Some other name";
            camera1->login = "Lexondry";
            camera1->password = "as65f6a4svsw56r7qrwfg456xcbn4d6";

            camera3->interaction_id = CameraInteraction::Axis;
            camera3->password = "ar42535464x153d4hd5345457654456";

            Change(adapter, { camera1, camera2, camera3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, cameras] = Get<CameraPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(cameras.size(), Is().EqualTo(3));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
            AssertThat(*cameras[1], Is().EqualTo(*camera2));
            AssertThat(*cameras[2], Is().EqualTo(*camera3));
        });

        it("Remove test", [&] {
            Remove(adapter, { camera3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, cameras] = Get<CameraPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(cameras.size(), Is().EqualTo(2));
            AssertThat(*cameras[0], Is().EqualTo(*camera1));
            AssertThat(*cameras[1], Is().EqualTo(*camera2));
        });
    });

    describe("Sqlite event statement test", [] {
        auto event1 = make_shared<Event>(Event{ "aso6d7821gfbiu", "Event 1", "Bad event", 1,
                                                "", "", 1576816, "",
                                                false, 534564, false, "", 0, "12dfasdf", false });
        auto event2 = make_shared<Event>(Event{ "23421rfvsdgzxg", "Event 2", "as22das", 1,
                                                "", "", 15768116, "",
                                                false, 534564, false, "", 0, "12dfasdf", true });
        auto event3 = make_shared<Event>(Event{ "sar13tgset1243", "Event 3", "asdas",  1,
                                                "", "", 15768163, "",
                                                false, 534564, false, "", 0, "12dfasdf", false });
        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<EventPtr>(adapter);
            Add(adapter, { event1, event2, event3 });
        });

        it("Get test", [&] {
            auto [status, events] = Get<EventPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(events.size(), Is().EqualTo(3));
            AssertThat(*events[0], Is().EqualTo(*event1));
            AssertThat(*events[1], Is().EqualTo(*event2));
            AssertThat(*events[2], Is().EqualTo(*event3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<EventPtr>(adapter);

            Add(adapter, { event1, event2, event3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, events] = Get<EventPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(events.size(), Is().EqualTo(3));
            AssertThat(*events[0], Is().EqualTo(*event1));
            AssertThat(*events[1], Is().EqualTo(*event2));
            AssertThat(*events[2], Is().EqualTo(*event3));
        });

        it("Change test", [&] {
            event1->name = "Some other name";
            event1->description = "AStgw3thk23htgsdgsdghsdgasdh";

            event2->processed = false;
            event2->created_time = 11;
            event2->processed_time = 12;
            event2->has_video = false;

            Change(adapter, { event1, event2, event3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, events] = Get<EventPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(events.size(), Is().EqualTo(3));
            AssertThat(*events[0], Is().EqualTo(*event1));
            AssertThat(*events[1], Is().EqualTo(*event2));
            AssertThat(*events[2], Is().EqualTo(*event3));
        });

        it("Remove test", [&] {
            Remove(adapter, { event2 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, events] = Get<EventPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(events.size(), Is().EqualTo(2));
            AssertThat(*events[0], Is().EqualTo(*event1));
            AssertThat(*events[1], Is().EqualTo(*event3));
        });
    });

    describe("Sqlite permission statement test", [] {
        auto perm1 = make_shared<Permission>(Permission{ "Cameras", "/cameras" });
        auto perm2 = make_shared<Permission>(Permission{ "Ptz control", "/./ptz" });
        auto perm3 = make_shared<Permission>(Permission{ "Test", "/./test/a124/^$&@$)" });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<PermissionPtr>(adapter);
            Add(adapter, { perm1, perm2, perm3 });
        });

        it("Get test", [&] {
            auto [status, perms] = Get<PermissionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(perms.size(), Is().EqualTo(3));
            AssertThat(*perms[0], Is().EqualTo(*perm1));
            AssertThat(*perms[1], Is().EqualTo(*perm2));
            AssertThat(*perms[2], Is().EqualTo(*perm3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<PermissionPtr>(adapter);
            Add(adapter, { perm1, perm2, perm3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, perms] = Get<PermissionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(perms.size(), Is().EqualTo(3));
            AssertThat(*perms[0], Is().EqualTo(*perm1));
            AssertThat(*perms[1], Is().EqualTo(*perm2));
            AssertThat(*perms[2], Is().EqualTo(*perm3));
        });

        it("Change test", [&] {
            perm1->name = "Some other name";
            perm1->regex_url = "125dsg23ygst23tyg";

            perm2->name = "12kj5479f8sdg82j39g";
            perm2->regex_url = "12kj5479f8sdg82j39g12kj5479f8sdg82j39g";

            Change(adapter, { perm1, perm2, perm3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, perms] = Get<PermissionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(perms.size(), Is().EqualTo(3));
            AssertThat(*perms[0], Is().EqualTo(*perm1));
            AssertThat(*perms[1], Is().EqualTo(*perm2));
            AssertThat(*perms[2], Is().EqualTo(*perm3));
        });

        it("Remove test", [&] {
            Remove(adapter, { perm2 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, perms] = Get<PermissionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(perms.size(), Is().EqualTo(2));
            AssertThat(*perms[0], Is().EqualTo(*perm1));
            AssertThat(*perms[1], Is().EqualTo(*perm3));
        });
    });

    describe("Sqlite settings statement test", [] {
        auto settings = make_shared<Settings>(Settings { {5, 9, 7, 21, 66, 1, 88, 2}, 50, 15, 16, 78 });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<SettingsPtr>(adapter);
            Add(adapter, { settings });
        });

        it("Get test", [&] {
            auto [status, setts] = Get<SettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(setts.size(), Is().EqualTo(1));
            AssertThat(*setts.front(), Is().EqualTo(*settings));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<SettingsPtr>(adapter);

            Add(adapter, { settings })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, setts] = Get<SettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(setts.size(), Is().EqualTo(1));
            AssertThat(*setts.front(), Is().EqualTo(*settings));
        });

        it("Change test", [&] {
            settings->registration = {4, 9, 7, 1, 6, 9};
            settings->ws_events_port = 54;
            settings->ws_camera_port = 99;
            settings->ws_model_port = 199;

            Change(adapter, { settings })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, setts] = Get<SettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(setts.size(), Is().EqualTo(1));
            AssertThat(*setts.front(), Is().EqualTo(*settings));
        });

        it("Remove test", [&] {
            Remove(adapter, { settings })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, setts] = Get<SettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(setts.size(), Is().EqualTo(0));
        });
    });

    describe("Sqlite user cameras statement test", [] {
        auto user_cameras1 = make_shared<UserCameras>(UserCameras {5, 6});
        auto user_cameras2 = make_shared<UserCameras>(UserCameras {1, 3});
        auto user_cameras3 = make_shared<UserCameras>(UserCameras {8, 4});

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserCamerasPtr>(adapter);
            Add(adapter, { user_cameras1, user_cameras2, user_cameras3 });
        });

        it("Get test", [&] {
            auto [status, user_cameras_list] = Get<UserCamerasPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_cameras_list.size(), Is().EqualTo(3));
            AssertThat(*user_cameras_list[0], Is().EqualTo(*user_cameras1));
            AssertThat(*user_cameras_list[1], Is().EqualTo(*user_cameras2));
            AssertThat(*user_cameras_list[2], Is().EqualTo(*user_cameras3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserCamerasPtr>(adapter);

            Add(adapter, { user_cameras1, user_cameras2, user_cameras3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, user_cameras_list] = Get<UserCamerasPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_cameras_list.size(), Is().EqualTo(3));
            AssertThat(*user_cameras_list[0], Is().EqualTo(*user_cameras1));
            AssertThat(*user_cameras_list[1], Is().EqualTo(*user_cameras2));
            AssertThat(*user_cameras_list[2], Is().EqualTo(*user_cameras3));
        });

        it("Remove test", [&] {
            Remove(adapter, { user_cameras1 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, user_cameras_list] = Get<UserCamerasPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_cameras_list.size(), Is().EqualTo(2));
            AssertThat(*user_cameras_list[0], Is().EqualTo(*user_cameras2));
            AssertThat(*user_cameras_list[1], Is().EqualTo(*user_cameras3));
        });
    });

    describe("Sqlite user permissions statement test", [] {
        auto user_perms1 = make_shared<UserPermissions>(UserPermissions {5, 6});
        auto user_perms2 = make_shared<UserPermissions>(UserPermissions {1, 3});
        auto user_perms3 = make_shared<UserPermissions>(UserPermissions {8, 4});

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserPermissionsPtr>(adapter);
            Add(adapter, { user_perms1, user_perms2, user_perms3 });
        });

        it("Get test", [&] {
            auto [status, user_perms_list] = Get<UserPermissionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_perms_list.size(), Is().EqualTo(3));
            AssertThat(*user_perms_list[0], Is().EqualTo(*user_perms1));
            AssertThat(*user_perms_list[1], Is().EqualTo(*user_perms2));
            AssertThat(*user_perms_list[2], Is().EqualTo(*user_perms3));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<UserPermissionsPtr>(adapter);

            Add(adapter, { user_perms1, user_perms2, user_perms3 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, user_perms_list] = Get<UserPermissionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_perms_list.size(), Is().EqualTo(3));
            AssertThat(*user_perms_list[0], Is().EqualTo(*user_perms1));
            AssertThat(*user_perms_list[1], Is().EqualTo(*user_perms2));
            AssertThat(*user_perms_list[2], Is().EqualTo(*user_perms3));
        });

        it("Remove test", [&] {
            Remove(adapter, { user_perms1 })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, user_perms_list] = Get<UserPermissionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(user_perms_list.size(), Is().EqualTo(2));
            AssertThat(*user_perms_list[0], Is().EqualTo(*user_perms2));
            AssertThat(*user_perms_list[1], Is().EqualTo(*user_perms3));
        });
    });

    describe("Sqlite sessions statement test", [] {
        auto session = make_shared<Sessions>(Sessions { "6s5ad1651fs3g56q4w", 5, 123456, 654987 });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<SessionsPtr>(adapter);
            Add(adapter, { session });
        });

        it("Get test", [&] {
            auto [status, sessions] = Get<SessionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(sessions.size(), Is().EqualTo(1));
            AssertThat(*sessions.front(), Is().EqualTo(*session));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<SessionsPtr>(adapter);

            Add(adapter, { session })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, sessions] = Get<SessionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(sessions.size(), Is().EqualTo(1));
            AssertThat(*sessions.front(), Is().EqualTo(*session));
        });

        it("Change test", [&] {
            session->user_id = 11;
            session->creation_time = 123;
            session->destruction_time = 321;

            Change(adapter, { session })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, sessions] = Get<SessionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(sessions.size(), Is().EqualTo(1));
            AssertThat(*sessions.front(), Is().EqualTo(*session));
        });

        it("Remove test", [&] {
            Remove(adapter, { session })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, sessions] = Get<SessionsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(sessions.size(), Is().EqualTo(0));
        });
    });

    describe("Sqlite camera action statement test", [] {
        auto camera_action = make_shared<CameraAction>(CameraAction { false, 5, "as56d464214",
                                                                      2165448l, "asd", 1 });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<CameraActionPtr>(adapter);
            Add(adapter, { camera_action });
        });

        it("Get test", [&] {
            auto [status, camera_actions] = Get<CameraActionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(camera_actions.size(), Is().EqualTo(1));
            AssertThat(*camera_actions.front(), Is().EqualTo(*camera_action));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<CameraActionPtr>(adapter);

            Add(adapter, { camera_action })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, camera_actions] = Get<CameraActionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(camera_actions.size(), Is().EqualTo(1));
            AssertThat(*camera_actions.front(), Is().EqualTo(*camera_action));
        });

        it("Change test", [&] {
            camera_action->succeed = true;
            camera_action->camera_id = 333;
            camera_action->session_hash = "%^&as645g652164w562";
            camera_action->time = 111;
            camera_action->action = "asd1as651s65b16wet4w23e5t";

            Change(adapter, { camera_action })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, camera_actions] = Get<CameraActionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(camera_actions.size(), Is().EqualTo(1));
            AssertThat(*camera_actions.front(), Is().EqualTo(*camera_action));
        });

        it("Remove test", [&] {
            Remove(adapter, { camera_action })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, camera_actions] = Get<CameraActionPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(camera_actions.size(), Is().EqualTo(0));
        });
    });

    describe("Sqlite event archive settings statement test", [] {
        auto event_set = make_shared<EventArchiveSettings>(EventArchiveSettings
                                                               { 1, "all;some;Cam1",
                                                                 5, 99 });

        auto db = make_shared<sqlite::database>(":memory:");
        auto adapter = make_shared<Adapter<sqlite::database>>(db);

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<EventArchiveSettingsPtr>(adapter);
            Add(adapter, { event_set });
        });

        it("Get test", [&] {
            auto [status, event_sets] = Get<EventArchiveSettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(event_sets.size(), Is().EqualTo(1));
            AssertThat(*event_sets.front(), Is().EqualTo(*event_set));
        });

        it("Add test", [&] {
            db = make_shared<sqlite::database>(":memory:");
            adapter = make_shared<Adapter<sqlite::database>>(db);
            CreateTable<EventArchiveSettingsPtr>(adapter);

            Add(adapter, { event_set })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, event_sets] = Get<EventArchiveSettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(event_sets.size(), Is().EqualTo(1));
            AssertThat(*event_sets.front(), Is().EqualTo(*event_set));
        });

        it("Change test", [&] {
            event_set->camera_id = 12;
            event_set->event_categories = "asd1as651s65b16wet4w23e5t";
            event_set->buffer_seconds = 321;
            event_set->record_seconds = 123;

            Change(adapter, { event_set })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, event_sets] = Get<EventArchiveSettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(event_sets.size(), Is().EqualTo(1));
            AssertThat(*event_sets.front(), Is().EqualTo(*event_set));
        });

        it("Remove test", [&] {
            Remove(adapter, { event_set })
                | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                | Check {};

            auto [status, event_sets] = Get<EventArchiveSettingsPtr>(adapter);
            status | OnError { [](auto& s) { throw std::runtime_error {s.error_message}; } }
                   | Check {};

            AssertThat(event_sets.size(), Is().EqualTo(0));
        });
    });
});