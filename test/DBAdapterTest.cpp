//
// Created by svakhreev on 27.12.16.
//

#include <memory>
#include <list>
#include <tuple>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <bandit/bandit.h>

#include "../source/database/adapters/SqliteAdapter.hpp"
#include "../source/database/adapters/SqlLanguageAdapter.hpp"

using namespace rttr;
using namespace bandit;

using std::list;
using std::vector;
using std::ostream;
using std::stringstream;
using std::tie;
using std::cout;
using std::endl;
using std::string;
using std::shared_ptr;
using boost::format;

using std::remove_reference_t;
using std::make_shared;
using std::tie;

using namespace cameramanagercore::database::adapters;

struct UserTest
{
    int age;
    string name;
    string password;
    double weight;
    int user_id;
};

bool operator==(const UserTest& lhs, const UserTest& rhs)
{
    return lhs.user_id == rhs.user_id
        && lhs.age == rhs.age
        && lhs.name == rhs.name
        && lhs.password == rhs.password
        && lhs.user_id == rhs.user_id;
}

struct CameraTest
{
    string name;
    string login;
    string password;
    int interaction_type;
    int camera_id;
};

bool operator==(const CameraTest& lhs, const CameraTest& rhs)
{
    return lhs.camera_id == rhs.camera_id
           && lhs.name == rhs.name
           && lhs.login == rhs.login
           && lhs.password == rhs.password
           && lhs.interaction_type == rhs.interaction_type;
}

RTTR_REGISTRATION
{
    registration::class_<UserTest>("UserTest")
        .constructor<>()
        .property("age",        &UserTest::age)
        .property("name",       &UserTest::name)
        .property("password",   &UserTest::password)
        .property("weight",     &UserTest::weight);

    registration::class_<CameraTest>("CameraTest")
        .constructor<>()
        .property("name",             &CameraTest::name)
        .property("login",            &CameraTest::login)
        .property("password",         &CameraTest::password)
        .property("interaction_type", &CameraTest::interaction_type);
}

go_bandit([]{
    describe("Select statement test", [] {
        auto db = make_shared<sqlite::database>(":memory:");

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");

            (*db) <<
                   "create table if not exists user ("
                       "   user_id integer primary key autoincrement not null,"
                       "   age int,"
                       "   name text,"
                       "   password text,"
                       "   weight real );";

            (*db) <<
                  "create table if not exists camera ("
                      "   camera_id integer primary key autoincrement not null,"
                      "   name text,"
                      "   login text,"
                      "   password text,"
                      "   interaction_type int );";
        });


        it("Select from request", [&]{
            auto st_1 = SelectFrom | UserTable;
            auto st_2 = SelectFrom | CameraTable;

            AssertThat(st_1,   Is().EqualTo("SELECT * FROM user"))
            AssertThat(st_2, Is().EqualTo("SELECT * FROM camera"))

            (*db) << st_1 >> []() {};
            (*db) << st_2 >> []() {};
        });

        it("SelectSome from request", [&]{
            auto st_1 = SelectSomeFrom("name, password, user_id")           | UserTable;
            auto st_2 = SelectSomeFrom("login, password, interaction_type") | CameraTable;

            AssertThat(st_1, Is().EqualTo("SELECT name, password, user_id FROM user"))
            AssertThat(st_2, Is().EqualTo("SELECT login, password, interaction_type FROM camera"))

            (*db) << st_1 >> []() {};
            (*db) << st_2 >> []() {};
        });

        it("SelectSome from where request", [&]{
            auto st_1 = SelectSomeFrom("name, password, user_id")           | UserTable    | Where("user_id = 6");
            auto st_2 = SelectSomeFrom("login, password, interaction_type") | CameraTable  | Where("camera_id = 1");

            AssertThat(st_1, Is().EqualTo("SELECT name, password, user_id FROM user WHERE user_id = 6;"))
            AssertThat(st_2, Is().EqualTo("SELECT login, password, interaction_type FROM camera WHERE camera_id = 1;"))

            (*db) << st_1 >> []() {};
            (*db) << st_2 >> []() {};
        });
    });

    describe("Insert statement test", [] {
        auto db = make_shared<sqlite::database>(":memory:");
        UserTest user { 20, "Sergey", "123456qwerty", 69.5 };
        CameraTest camera { "CAM1", "admin", "admin", 1 };

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");

            (*db) <<
                  "create table if not exists user ("
                      "   user_id integer primary key autoincrement not null,"
                      "   age int,"
                      "   name text,"
                      "   password text,"
                      "   weight real );";

            (*db) <<
                  "create table if not exists camera ("
                      "   camera_id integer primary key autoincrement not null,"
                      "   name text,"
                      "   login text,"
                      "   password text,"
                      "   interaction_type int );";

        });

        it("InsertTo request", [&]{
            auto st_1 = InsertTo | UserTable   | Values(user);
            auto st_2 = InsertTo | CameraTable | Values(camera);

            AssertThat(st_1, Is().EqualTo("INSERT INTO user (age, name, password, weight) "
                                          "VALUES (20, 'Sergey', '123456qwerty', 69.5);"));
            AssertThat(st_2, Is().EqualTo("INSERT INTO camera (name, login, password, interaction_type) "
                                          "VALUES ('CAM1', 'admin', 'admin', 1);"));

            (*db) << st_1;
            (*db) << st_2;
        });

        it("InsertTo with custom values request", [&]{
            auto st_1 = InsertTo | UserTable   | ValuesManual("(name, password) VALUES (\"admin\", \"admin\")");
            auto st_2 = InsertTo | CameraTable | ValuesManual("(name, password) VALUES (\"admin\", \"admin\")");

            AssertThat(st_1, Is().EqualTo("INSERT INTO user (name, password) "
                                          "VALUES (\"admin\", \"admin\");"));
            AssertThat(st_2, Is().EqualTo("INSERT INTO camera (name, password) "
                                          "VALUES (\"admin\", \"admin\");"));

            (*db) << st_1;
            (*db) << st_2;
        });
    });

    describe("Update statement test", [] {
        auto db = make_shared<sqlite::database>(":memory:");
        UserTest user { 20, "Sergey", "123456qwerty", 69.5 };
        CameraTest camera { "CAM1", "admin", "admin", 1 };

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");

            (*db) <<
                  "create table if not exists user ("
                      "   user_id integer primary key autoincrement not null,"
                      "   age int,"
                      "   name text,"
                      "   password text,"
                      "   weight real );";

            (*db) <<
                  "create table if not exists camera ("
                      "   camera_id integer primary key autoincrement not null,"
                      "   name text,"
                      "   login text,"
                      "   password text,"
                      "   interaction_type int );";

        });

        it("Update request", [&]{
            auto st_1 = Update | UserTable   | From(user)   | Where("user_id = 0");
            auto st_2 = Update | CameraTable | From(camera) | Where("camera_id = 0");

            AssertThat(st_1, Is().EqualTo("UPDATE user SET "
                                          "age = 20, name = 'Sergey', password = '123456qwerty', weight = 69.5 "
                                          "WHERE user_id = 0;"));
            AssertThat(st_2, Is().EqualTo("UPDATE camera SET "
                                          "name = 'CAM1', login = 'admin', password = 'admin', interaction_type = 1 "
                                          "WHERE camera_id = 0;"));

            (*db) << st_1;
            (*db) << st_2;
        });

        it("Update with custom values request", [&]{
            auto st_1 = Update | UserTable   | FromManual("name = 'adminis', password = '12'") | Where("name = 'admin'");
            auto st_2 = Update | CameraTable | FromManual("name = 'adminis', password = '12'") | Where("name = 'admin'");

            AssertThat(st_1, Is().EqualTo("UPDATE user SET "
                                          "name = 'adminis', password = '12' "
                                          "WHERE name = 'admin';"));
            AssertThat(st_2, Is().EqualTo("UPDATE camera SET "
                                          "name = 'adminis', password = '12' "
                                          "WHERE name = 'admin';"));

            (*db) << st_1;
            (*db) << st_2;
        });
    });

    describe("Delete statement test", [] {
        auto db = make_shared<sqlite::database>(":memory:");
        UserTest user { 20, "Sergey", "123456qwerty", 69.5 };
        CameraTest camera { "CAM1", "admin", "admin", 1 };

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");

            (*db) <<
                  "create table if not exists user ("
                      "   user_id integer primary key autoincrement not null,"
                      "   age int,"
                      "   name text,"
                      "   password text,"
                      "   weight real );";

            (*db) <<
                  "create table if not exists camera ("
                      "   camera_id integer primary key autoincrement not null,"
                      "   name text,"
                      "   login text,"
                      "   password text,"
                      "   interaction_type int );";

        });

        it("Delete request", [&]{
            auto st_1 = DeleteFrom | UserTable   | Where(get_values_whereand(user));
            auto st_2 = DeleteFrom | CameraTable | Where(get_values_whereand(camera));

            AssertThat(st_1, Is().EqualTo("DELETE FROM user "
                                          "WHERE age = 20 AND name = 'Sergey' AND password = '123456qwerty' AND weight = 69.5;"));
            AssertThat(st_2, Is().EqualTo("DELETE FROM camera "
                                          "WHERE name = 'CAM1' AND login = 'admin' AND password = 'admin' AND interaction_type = 1;"));

            (*db) << st_1;
            (*db) << st_2;
        });

        it("Delete with custom values request", [&]{
            auto st_1 = DeleteFrom | UserTable   | Where("user_id = 0");
            auto st_2 = DeleteFrom | CameraTable | Where("camera_id = 0");

            AssertThat(st_1, Is().EqualTo("DELETE FROM user "
                                          "WHERE user_id = 0;"));
            AssertThat(st_2, Is().EqualTo("DELETE FROM camera "
                                          "WHERE camera_id = 0;"));

            (*db) << st_1;
            (*db) << st_2;
        });
    });

    describe("Work with adapter test", [] {
        auto db = make_shared<sqlite::database>(":memory:");

        UserTest user1 { 25, "Mike",   "12345", 84.5 };
        UserTest user2 { 32, "Sergey", "65721", 80.5 };
        UserTest user3 { 22, "Alex",   "65654", 82.3 };
        CameraTest camera1 { "CAM1", "admin", "admin", 1 };
        CameraTest camera2 { "CAM2", "admin", "564212", 1 };
        CameraTest camera3 { "CAM3", "admin", "12efsa", 0 };

        bandit::before_each([&]{
            db = make_shared<sqlite::database>(":memory:");

            (*db) <<
                  "create table if not exists user ("
                      "   user_id integer primary key autoincrement not null,"
                      "   age int,"
                      "   name text,"
                      "   password text,"
                      "   weight real );";

            (*db) <<
                  "create table if not exists camera ("
                      "   camera_id integer primary key autoincrement not null,"
                      "   name text,"
                      "   login text,"
                      "   password text,"
                      "   interaction_type int );";

        });

        it("Users test", [&]{
            Adapter<sqlite::database> adapter { db };

            adapter << (InsertTo   | UserTable | Values(user1))
                    >> ExecuteEmpty{}
                    << (SelectFrom | UserTable | WhereRowLike(user1))
                    >> tie(user1.user_id)

                    << (InsertTo   | UserTable | Values(user2))
                    >> ExecuteEmpty{}
                    << (SelectFrom | UserTable | WhereRowLike(user2))
                    >> tie(user2.user_id)

                    << (InsertTo   | UserTable | Values(user3))
                    >> ExecuteEmpty{}
                    << (SelectFrom | UserTable | WhereRowLike(user3))
                    >> tie(user3.user_id);

            AssertThat(user1.user_id, Is().Not().EqualTo(0));
            AssertThat(user2.user_id, Is().Not().EqualTo(0));
            AssertThat(user3.user_id, Is().Not().EqualTo(0));

            vector<UserTest> users;
            adapter << (SelectFrom | UserTable)
                    >> [&](int user_id,
                           int age,
                           string name,
                           string password,
                           double weight) { users.emplace_back(UserTest { age, name, password, weight, user_id }); };
            AssertThat(users.size(), Is().EqualTo(3))
            AssertThat(users[0], Is().EqualTo(user1));
            AssertThat(users[1], Is().EqualTo(user2));
            AssertThat(users[2], Is().EqualTo(user3));


            user1.name = "Artur";
            user1.password = "Fdasfasfg84";
            adapter << (Update | UserTable | From(user1) | Where((format("user_id = %1%") % user1.user_id).str()))
                    >> ExecuteEmpty{};

            adapter << (DeleteFrom | UserTable | Where((format("user_id = %1%") % user2.user_id).str()))
                    >> ExecuteEmpty{};

            users.clear();
            adapter << (SelectFrom | UserTable)
                    >> [&](int user_id,
                           int age,
                           string name,
                           string password,
                           double weight) { users.emplace_back(UserTest { age, name, password, weight, user_id }); };
            AssertThat(users.size(), Is().EqualTo(2))
            AssertThat(users[0], Is().EqualTo(user1));
            AssertThat(users[1], Is().EqualTo(user3));

            AssertThat(adapter.GetCommandLog()->size(), Is().EqualTo(10));
        });

        it("Cameras test", [&]{
            Adapter<sqlite::database> adapter { db };

            adapter << (InsertTo   | CameraTable | Values(camera1))
                    >> ExecuteEmpty{}
                    << (SelectFrom | CameraTable | WhereRowLike(camera1))
                    >> tie(camera1.camera_id)

                    << (InsertTo   | CameraTable | Values(camera2))
                    >> ExecuteEmpty{}
                    << (SelectFrom | CameraTable | WhereRowLike(camera2))
                    >> tie(camera2.camera_id)

                    << (InsertTo   | CameraTable | Values(camera3))
                    >> ExecuteEmpty{}
                    << (SelectFrom | CameraTable | WhereRowLike(camera3))
                    >> tie(camera3.camera_id);

            AssertThat(camera1.camera_id, Is().Not().EqualTo(0));
            AssertThat(camera2.camera_id, Is().Not().EqualTo(0));
            AssertThat(camera3.camera_id, Is().Not().EqualTo(0));

            vector<CameraTest> cameras;
            adapter << (SelectFrom | CameraTable)
                    >> [&](int camera_id,
                           string name,
                           string login,
                           string password,
                           int type) { cameras.emplace_back(CameraTest { name, login, password, type, camera_id }); };
            AssertThat(cameras.size(), Is().EqualTo(3))
            AssertThat(cameras[0], Is().EqualTo(camera1));
            AssertThat(cameras[1], Is().EqualTo(camera2));
            AssertThat(cameras[2], Is().EqualTo(camera3));


            camera1.name = "SUPER CAMERA 1";
            camera1.login = "a2s6qw7t86qwf4as23fv41s2d3g4s6eg7";
            camera1.password = "xcbrtjktfga5s7r24rfv51scb2df468344";
            adapter << (Update | CameraTable | From(camera1) | Where((format("camera_id = %1%") % camera1.camera_id).str()))
                    >> ExecuteEmpty{};

            adapter << (DeleteFrom | CameraTable | Where((format("camera_id = %1%") % camera2.camera_id).str()))
                    >> ExecuteEmpty{};

            cameras.clear();
            adapter << (SelectFrom | CameraTable)
                    >> [&](int camera_id,
                           string name,
                           string login,
                           string password,
                           int type) { cameras.emplace_back(CameraTest { name, login, password, type, camera_id }); };
            AssertThat(cameras.size(), Is().EqualTo(2))
            AssertThat(cameras[0], Is().EqualTo(camera1));
            AssertThat(cameras[1], Is().EqualTo(camera3));

            AssertThat(adapter.GetCommandLog()->size(), Is().EqualTo(10));
        });
    });
});