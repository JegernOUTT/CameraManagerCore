//
// Created by svakhreev on 07.02.17.
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
#include <sstream>
#include <map>

#include <bandit/bandit.h>
#include <range/v3/all.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <nlohmann/json.hpp>

#include "../source/iss/IssConnection.hpp"

using namespace bandit;
using namespace ranges;
using namespace std::chrono;
using namespace cameramanagercore::iss;

using std::move;
using std::make_unique;
using std::string;
using std::map;

using nlohmann::json;

go_bandit([] {
    describe("Rest testing", [] {

        it_skip("GetCamerasAllInformation", [&] {
            IssConnection<int> connection("10.10.20.10:8888", "admin", "admin");
            auto result = connection.SendRequest(ApiPath::GetCamerasAllInformation);
            AssertThat(static_cast<bool>(result), Is().True());
        });

        it_skip("GetCamerasConnectionInformation", [&] {
            IssConnection<int> connection("10.10.20.10:8888", "admin", "admin");
            auto result = connection.SendRequest(ApiPath::GetCamerasConnectionInformation);
            AssertThat(static_cast<bool>(result), Is().True());
        });

        it_skip("GetRtspInformation", [&] {
            IssConnection<int> connection("10.10.20.10:8888", "admin", "admin");
            auto result = connection.SendRequest(ApiPath::GetCameraRtspInformation, 3);
            AssertThat(static_cast<bool>(result), Is().True());
        });

        it_skip("GetUsersInformation", [&] {
            IssConnection<int> connection("10.10.20.10:8888", "admin", "admin");
            auto result = connection.SendRequest(ApiPath::GetUsersInformation);
            AssertThat(static_cast<bool>(result), Is().True());
        });

        it_skip("GetCameraEvents", [&] {
            IssConnection<int> connection("10.10.20.10:8888", "admin", "admin");
            using namespace boost::posix_time;
            using namespace boost::gregorian;

            auto result = connection.SendRequest(ApiPath::GetCameraEvents,
                                                 2,
                                                 to_iso_string(ptime(date(2017, Jan, 1))),
                                                 "5");
//            std::cout << result.value().dump(2) << std::endl;
//            AssertThat(static_cast<bool>(result), Is().True());
        });
    });

});