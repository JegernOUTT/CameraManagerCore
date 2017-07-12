//
// Created by svakhreev on 10.01.17.
//
#include <string>
#include <cassert>
#include <type_traits>
#include <functional>
#include <memory>
#include <future>
#include <chrono>
#include <thread>
#include <vector>

#include <bandit/bandit.h>
#include <range/v3/all.hpp>
#include <boost/thread.hpp>

#include "../source/utility/RandomGenerator.hpp"
#include "../source/thread_pool/pool/ThreadPool.hpp"

using namespace std::chrono;
using namespace std::literals;
using namespace std::this_thread;
using namespace bandit;
using namespace ranges;
using namespace cameramanagercore::thread_pool;

using std::string;
using std::packaged_task;
using std::future;
using std::vector;
using std::thread;

using std::move;
using std::make_unique;

int test_free_func(int i)
{
    return i;
}

template <typename T>
T test_free_func_template(T p)
{
    return p;
}

void test_void(int &p, int v)
{
    p = v;
}

struct A
{
    int b(const int &p)
    {
        return p;
    }

    void c(int &i)
    {
        i = 43;
    }
};

template <typename T>
struct Foo
{
    template <typename U>
    U bar(U p)
    {
        return p + payload;
    }

    T payload;
};

template <typename T>
void print_overhead()
{
    using func_type = FixedFunction<void(), sizeof(T)>;
    int t_s = sizeof(T);
    int f_s = sizeof(func_type);
    std::cout << " - for type size " << t_s << "\n"
              << "    function size is " << f_s << "\n"
              << "    overhead is " << float(f_s - t_s)/t_s * 100 << "%\n";
}

static string str_fun()
{
    return "123";
}


go_bandit([] {
    describe("Fixed function tests", [] {
        it("alloc/dealloc", [] {
            static size_t def = 0;
            static size_t cop = 0;
            static size_t mov = 0;
            static size_t cop_ass = 0;
            static size_t mov_ass = 0;
            static size_t destroyed = 0;
            struct cnt
            {
                string payload;
                cnt() { def++; }
                cnt(const cnt& o) { payload = o.payload; cop++;}
                cnt(cnt&& o) { payload = std::move(o.payload); mov++;}
                cnt& operator=(const cnt& o) { payload = o.payload; cop_ass++; return *this; }
                cnt& operator=(cnt&& o) { payload = std::move(o.payload); mov_ass++; return *this; }
                ~cnt() { destroyed++; }
                std::string operator()() { return payload; }
            };

            {
                cnt c1;
                c1.payload = "xyz";
                FixedFunction<string()> f1(c1);
                AssertThat("xyz"s, Is().EqualTo(f1()));

                FixedFunction<string()> f2;
                f2 = std::move(f1);
                AssertThat("xyz"s, Is().EqualTo(f2()));

                FixedFunction<string()> f3(std::move(f2));
                AssertThat("xyz"s, Is().EqualTo(f3()));

                FixedFunction<string()> f4(str_fun);
                AssertThat("123"s, Is().EqualTo(f4()));

                f4 = std::move(f3);
                AssertThat("xyz"s, Is().EqualTo(f4()));

                cnt c2;
                c2.payload = "qwe";
                f4 = std::move(FixedFunction<string()>(c2));
                AssertThat(string("qwe"), Is().EqualTo(f4()));
            }

            AssertThat(def + cop + mov, Is().EqualTo(destroyed));
            AssertThat(def, Is().EqualTo(2));
            AssertThat(cop, Is().EqualTo(0));
            AssertThat(mov, Is().EqualTo(6));
            AssertThat(cop_ass, Is().EqualTo(0));
            AssertThat(mov_ass, Is().EqualTo(0));
        });

        it("free func", [] {
            FixedFunction<int(int)> f(test_free_func);
            AssertThat(f(3), Is().EqualTo(3));
        });

        it("free func template", [] {
            FixedFunction<string(string)> f(test_free_func_template<string>);
            AssertThat(f("abc"), Is().EqualTo("abc"s));
        });


        it("void func", [] {
            FixedFunction<void(int&, int)> f(test_void);
            int p = 0;
            f(p, 42);
            AssertThat(p, Is().EqualTo(42));
        });

        it("class method void", [] {
            using namespace std::placeholders;
            A a;
            int i = 0;
//            FixedFunction<void(int &)> f(std::bind(&A::c, &a, std::placeholders::_1));
//            f(i);
//            AssertThat(i, Is().EqualTo(43));
        });

        it("class method 1", [] {
            using namespace std::placeholders;
            A a;
//            FixedFunction<int(const int&)> f(std::bind(&A::b, &a, std::placeholders::_1));
//            AssertThat(f(4), Is().EqualTo(4));
        });

        it("class method 2", [] {
            using namespace std::placeholders;
//            Foo<float> foo;
//            foo.payload = 1.f;
//            FixedFunction<int(int)> f(std::bind(&Foo<float>::bar<int>, &foo, std::placeholders::_1));
//            AssertThat(f(1), Is().EqualTo(2));
        });

        it("lambda", [] {
            const string s1 = "s1";
            FixedFunction<string()> f([&s1]() {
                return s1;
            });
            AssertThat(f(), Is().EqualTo(s1));
        });
    });

    describe("Thread pool test", [] {
        using ThreadPoolStd = ThreadPool<>;

        it("Post job", [] {
            ThreadPoolStd pool;
            packaged_task<int()> t([]()
                                   {
                                       sleep_for(1ms);
                                       return 42;
                                   });

            future<int> r = t.get_future();
            pool.post(t);
            AssertThat(r.get(), Is().EqualTo(42));
        });

        it("Test exception", [] {
            ThreadPoolStd pool;
            packaged_task<void()> t([]()
                                   {
                                       sleep_for(RandomGenerator::RandomDuration<milliseconds>(500, 2500));
                                       throw std::exception {};
                                   });

            future<void> f = t.get_future();
            pool.post(t);
            AssertThrows(std::exception, f.get());
        });

        it("Stress test", [] {
            auto pool = make_unique<ThreadPoolStd>();

            const size_t iter_count = 1000;
            auto v = make_unique<vector<future<int>>>();
            v->reserve(iter_count);

            for (auto stress_iteration: view::ints | view::take(10))
            {
                for (auto repeat: view::ints | view::take(iter_count))
                {
                    packaged_task<int()> t([]()
                                           {
                                               sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 20));
                                               return 42;
                                           });

                    v->emplace_back(std::move(t.get_future()));
                    pool->post(t);
                }

                for (auto& f: *v)
                AssertThat(f.get(), Is().EqualTo(42));
                v->clear();
            }
        });

        it("Multithreaded std stress test", [] {
            auto pool = make_unique<ThreadPoolStd>();
            auto threads = make_unique<vector<thread>>();
            threads->reserve(8);

            for (auto stress_iteration: view::ints | view::take(1000))
            {
                for (auto repeat: view::ints | view::take(8))
                {
                    threads->emplace_back(thread{[&] {
                        packaged_task<int()> t([]() {
                            sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 20));
                            return 42;
                        });
                        auto f = t.get_future();
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                        pool->post(t);
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                        AssertThat(f.get(), Is().EqualTo(42));
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                    }});
                }

                for (auto& t: *threads)
                    t.join();
                threads->clear();
            }
        });

        it("Multithreaded boost stress test", [] {
            auto pool = make_unique<ThreadPoolStd>();
            auto threads = make_unique<vector<boost::thread>>();
            threads->reserve(8);

            for (auto stress_iteration: view::ints | view::take(1000))
            {
                for (auto repeat: view::ints | view::take(8))
                {
                    threads->emplace_back(boost::thread{[&] {
                        boost::packaged_task<int> t([]() {
                            sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 20));
                            return 42;
                        });
                        auto f = t.get_future();
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                        pool->post(t);
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                        AssertThat(f.get(), Is().EqualTo(42));
                        sleep_for(RandomGenerator::RandomDuration<milliseconds>(2, 10));
                    }});
                }

                for (auto& t: *threads)
                    t.join();
                threads->clear();
            }
        });
    });
});