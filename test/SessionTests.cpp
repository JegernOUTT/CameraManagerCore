//
// Created by svakhreev on 26.12.16.
//

#include <unordered_set>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <thread>
#include <string_view>

#include <bandit/bandit.h>
#include <range/v3/all.hpp>

#include "../source/utility/RandomGenerator.hpp"

#include "../source/model/User.hpp"
#include "../source/model/ModelContext.hpp"
#include "../source/signals/SignalContext.hpp"
#include "../source/session/SecreteKeyGenerator.hpp"
#include "../source/session/Session.hpp"
#include "../source/session/SessionContext.hpp"

using namespace bandit;
using namespace ranges::v3;
using namespace std::chrono;
using namespace std::this_thread;
using namespace cameramanagercore::session;
using namespace cameramanagercore::model;

using std::unordered_set;
using std::string;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::vector;
using std::thread;
using std::string_view;

using std::isalpha;
using std::isdigit;
using std::make_shared;
using std::make_unique;

using Generator = SecreteKeyGenerator<random_alg>;
using UserPtr = shared_ptr<User>;
using SessionSpec = Session<Generator, seconds, UserPtr>;
using SessionSpecPtr = shared_ptr<Session<Generator, seconds, UserPtr>>;
using SessionContextLazy = SessionContext<SessionSpecPtr, seconds, SessionClearPolicy::Lazy, Model>;
using SessionContextAsync = SessionContext<SessionSpecPtr, seconds, SessionClearPolicy::Async, Model>;

go_bandit([]{
    describe("Secrete key test", []{
        Generator generator;

        it("Check secret key generation", [&]{
            for (int i = 0; i < 100; ++i)
            {
                auto key = generator();

                AssertThat(key.size(), Is().GreaterThanOrEqualTo(20));
                auto chars_count = distance(key | view::filter([](auto &item) { return isalpha(item); }));
                AssertThat(chars_count, Is().GreaterThanOrEqualTo(1));

                auto digits_count = distance(key | view::filter([](auto &item) { return isdigit(item); }));
                AssertThat(digits_count, Is().GreaterThanOrEqualTo(1));
            }
        });

        it("Check secret key generation uniqueness", [&]{
            unordered_set<string> keys;

            for (int i = 0; i < 100; ++i)
            {
                auto key = generator();

                auto iter = keys.find(key);
                AssertThat(iter, Is().EqualTo(keys.end()));

                keys.insert(key);
            }
        });
    });

    describe("Session timeout test", []{
        auto user = make_shared<User>(User{"admin", "admin"});

        it("1 second timeout test", [&]{
            auto time = 1s;

            SessionSpec expired_session { time, user };
            sleep_for(2s);
            AssertThat(expired_session.Status(), Is().EqualTo(SessionStatus::Expired));

            expired_session = SessionSpec{ time, user };
            sleep_for(50ms);
            AssertThat(expired_session.Status(), Is().EqualTo(SessionStatus::Living));
        });

        it("2 second timeout test", [&]{
            auto time = 2s;

            auto expired_session = SessionSpec{ time, user };
            sleep_for(5s);
            AssertThat(expired_session.Status(), Is().EqualTo(SessionStatus::Expired));

            expired_session = SessionSpec{ time, user };
            sleep_for(500ms);
            AssertThat(expired_session.Status(), Is().EqualTo(SessionStatus::Living));
        });
    });

    describe("Session context", []{
        auto user = make_shared<User>(User{"admin", "admin"});

        it("Add one session", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextLazy context(model);
            auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(100, 200), user });
            context.AddSession(session);
            AssertThat(context.GetSessions().front(), Is().EqualTo(session));
        });

        it("Find session", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextLazy context(model);
            auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(100, 200), user });
            context.AddSession(session);

            for (int i = 0; i < 1000; ++i)
            {
                auto session_c = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(100, 200), user });
                context.AddSession(session_c);
            }

            auto finded = context.FindSession(session->Secrete());

            AssertThat(finded, Is().EqualTo(session));
        });

        it("Add sessions to lazy remove context", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextLazy context(model);

            for (int i = 0; i < 1000; ++i)
            {
                auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(5, 10), user });
                context.AddSession(session);
            }

            sleep_for(12s);
            AssertThat(context.GetSessions().size(), Is().EqualTo(0));

            for (int i = 0; i < 1000; ++i)
            {
                auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(1000, 10000), user });
                context.AddSession(session);
            }

            sleep_for(10s);
            AssertThat(context.GetSessions().size(), Is().EqualTo(1000));
        });

        it("Add sessions to remove context", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextAsync context { model, 1s };

            for (int i = 0; i < 1000; ++i)
            {
                auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(5, 10), user });
                context.AddSession(session);
            }

            sleep_for(121s);
            AssertThat(context.GetSessions().size(), Is().EqualTo(0));

            for (int i = 0; i < 1000; ++i)
            {
                auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(1000, 10000), user });
                context.AddSession(session);
            }

            sleep_for(10s);
            AssertThat(context.GetSessions().size(), Is().EqualTo(1000));
        });

        it("Multithreaded async tests with random pauses", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextAsync context { model, 1s };

            vector<unique_ptr<thread>> v;
            v.reserve(8);
            for (int i = 0; i < 4; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(500, 1000),
                                                                                                                user });
                                                           sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                           context.AddSession(session);
                                                       }
                                                   }));
           for (int i = 0; i < 2; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto s = context.GetSessions();
                                                           sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                       }
                                                   }));
            for (int i = 0; i < 2; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto sessions = context.GetSessions();
                                                           for (auto& s: sessions)
                                                           {
                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                               AssertThat(context.FindSession(s->GetUser()), Is().EqualTo(s));

                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));

                                                               AssertThat(context.FindSession(s->Secrete()), Is().EqualTo(s));
                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                           }
                                                       }
                                                   }));

            for (auto& th: v)
                th->join();
        });

        it("Multithreaded lazy tests with random pauses", [&]{
            auto model = make_shared<Model>(make_shared<cameramanagercore::signals::SignalsContext>());
            SessionContextLazy context { model };

            vector<unique_ptr<thread>> v;
            v.reserve(8);
            for (int i = 0; i < 4; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto session = make_shared<SessionSpec>(SessionSpec{ RandomGenerator::RandomDuration<seconds>(500, 1000),
                                                                                                                user });
                                                           sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                           context.AddSession(session);
                                                       }
                                                   }));
            for (int i = 0; i < 2; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto s = context.GetSessions();
                                                           sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                       }
                                                   }));
            for (int i = 0; i < 2; ++i)
                v.emplace_back(make_unique<thread>([&]
                                                   {
                                                       for (auto repeat: view::ints | view::take(100))
                                                       {
                                                           auto sessions = context.GetSessions();
                                                           for (auto& s: sessions)
                                                           {
                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                               AssertThat(context.FindSession(s->GetUser()), Is().EqualTo(s));

                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));

                                                               AssertThat(context.FindSession(s->Secrete()), Is().EqualTo(s));
                                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(1, 10));
                                                           }
                                                       }
                                                   }));

            for (auto& th: v)
                th->join();
        });
    });
});