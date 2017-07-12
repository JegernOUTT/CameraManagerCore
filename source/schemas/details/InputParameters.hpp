//
// Created by svakhreev on 25.03.17.
//

#ifndef CAMERAMANAGERCORE_INPUTPARAMETERS_HPP
#define CAMERAMANAGERCORE_INPUTPARAMETERS_HPP

#include <tuple>
#include <unordered_map>
#include <tuple>
#include <optional>

#include <nlohmann/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "./ResponseBuilder.hpp"
#include "../../utility/LoggerSettings.hpp"
#include "../../utility/TupleUtility.hpp"

namespace cameramanagercore::schemas
{

template <typename... Output, typename... Input, std::size_t... I>
constexpr auto apply_cast_impl(std::tuple<Input...> t, std::index_sequence<I...>)
{
    return std::make_tuple(boost::lexical_cast< typename std::tuple_element<I, std::tuple<Output...> >::type >
                               (std::get<I>(t))...);
}

template <typename... Output, typename... Input>
constexpr auto apply_cast(std::tuple<Input...> t)
{
    return apply_cast_impl<Output...>(t, std::make_index_sequence<sizeof...(Output)>{});
}

template <size_t I, typename... Output, typename... Input, typename... Temp>
constexpr auto apply_optional_cast_impl(std::tuple<std::optional<Input>...> input,
                                        std::tuple<std::optional<Temp>...> output)
{
    using CurrentOutputType = typename std::tuple_element<I, std::tuple<Output...>>::type;

    if constexpr (I >= (sizeof...(Input) - 1))
    {
        if (std::get<I>(input)) {
            auto value = boost::lexical_cast<CurrentOutputType>(std::get<I>(input).value());
            return std::tuple_cat(output, std::make_tuple(std::make_optional(value)));
        }
        else
        {
            return std::tuple_cat(output, std::make_tuple(std::optional<CurrentOutputType>{}));
        }
    }
    else
    {
        if (std::get<I>(input)) {
            auto value = boost::lexical_cast<CurrentOutputType>(std::get<I>(input).value());
            return apply_optional_cast_impl<I + 1, Output...>(input,
                                                              std::tuple_cat(output,
                                                                             std::make_tuple(std::make_optional(value))));
        }
        else
        {
            return apply_optional_cast_impl<I + 1, Output...>(input,
                                                              std::tuple_cat(output,
                                                                             std::make_tuple(
                                                                                 std::optional<CurrentOutputType>{})));
        }
    }
}

template <typename... Output, typename... Input>
constexpr auto apply_optional_cast(std::tuple<std::optional<Input>...> input)
{
    using CurrentOutputType = typename std::tuple_element<0, std::tuple<Output...>>::type;

    if (std::get<0>(input))
    {
        auto value = boost::lexical_cast<CurrentOutputType>(std::get<0>(input).value());
        return apply_optional_cast_impl<1, Output...>(input,
                                                      std::make_tuple(std::make_optional(value)));
    }
    else
    {
        return apply_optional_cast_impl<1, Output...>(input,
                                                      std::make_tuple(std::optional<CurrentOutputType>{}));
    }
}

template <typename... Output, typename Request, typename... Parameters>
std::pair< std::string, std::optional< std::tuple<Output...> > >
get_from_header(Request request, Parameters... parameters)
{
    using namespace std;
    using namespace nlohmann;
    using namespace cameramanagercore::utility;

    static_assert(sizeof...(Parameters) == sizeof...(Output), "Size of output parameters must be same with Input");

    try
    {
        auto requests = make_tuple(make_tuple(parameters, request->header.find(parameters))...);
        tuple_visit(requests, [&request] (auto r) { if (std::get<1>(r) == request->header.end())
                                                        throw std::invalid_argument { std::get<0>(r) }; });

        auto transformed = tuple_transform(requests, [](auto r) { return std::get<1>(r)->second; });

        auto fields_s = ((parameters + ";"s) + ...);
        auto params_s = tuple_fold(transformed, ""s, [](std::string s, auto element) { return s += (element + ";"); });
        LOG(info) << boost::format("Founded request parameters for fields: %1%. With parameters: %2%.")
                     % fields_s % params_s;

        return {params_s,  apply_cast<Output...>(transformed)};
    }
    catch (std::invalid_argument& e)
    {
        auto error_text = boost::format("Can not find input parameters: %1%") % e.what();
        LOG(error) << error_text;
        return {error_text.str(), {}};
    }
    catch (...)
    {
        auto error_text =  "Some error while input parameters parsing";
        LOG(error) << error_text;
        return {error_text, {}};
    }

    return {};
}

template <typename Request, typename Type>
std::pair< std::string, std::optional< Type > >
get_from_body(Request request)
{
    using namespace std;
    using namespace nlohmann;
    using namespace cameramanagercore::utility;

    try
    {
        auto str = request->content.string();
        LOG(error) << "AAA: " << str;
        return {"", Type { nlohmann::json::parse(str) }};
    }
    catch (std::exception& e)
    {
        auto error_text = boost::format("Error while json parsing: %1%") % e.what();
        LOG(error) << error_text;
        return {error_text.str(), {}};
    }
    catch (...)
    {
        auto error_text =  "Unknown error while json parsing";
        LOG(error) << error_text;
        return {error_text, {}};
    }

    return {};
}

template < typename Request >
std::pair< std::string, std::optional< nlohmann::json > >
get_from_body(Request request)
{
    using namespace std;
    using namespace nlohmann;
    using namespace cameramanagercore::utility;

    try
    {
        auto str = request->content.string();
        LOG(error) << "AAA: " << str;
        return {"", nlohmann::json::parse(str)};
    }
    catch (std::exception& e)
    {
        auto error_text = boost::format("Error while json parsing: %1%") % e.what();
        LOG(error) << error_text;
        return {error_text.str(), {}};
    }
    catch (...)
    {
        auto error_text =  "Unknown error while json parsing";
        LOG(error) << error_text;
        return {error_text, {}};
    }

    return {};
}

template <typename... Output, typename... Parameters>
std::pair< std::string, std::optional< std::tuple<Output...> > >
get_from_url(const std::unordered_map<std::string, std::string>& request, Parameters... parameters)
{
    using namespace std;
    using namespace nlohmann;
    using namespace cameramanagercore::utility;

    static_assert(sizeof...(Parameters) == sizeof...(Output), "Size of output parameters must be same with Input");

    try
    {
        auto requests = make_tuple(make_tuple(parameters, request.find(parameters))...);
        tuple_visit(requests, [&request] (auto r) { if (std::get<1>(r) == request.end())
                                                    throw std::invalid_argument { std::get<0>(r) }; });

        auto transformed = tuple_transform(requests, [](auto r) { return std::get<1>(r)->second; });

        auto fields_s = ((parameters + ";"s) + ...);
        auto params_s = tuple_fold(transformed, ""s,
                                   [](std::string s, auto element) { return s += (element + ";"); });
        LOG(info) << boost::format("Finded request parameters for such fields: %1%. With parameters: %2%.")
                     % fields_s % params_s;

        return {params_s,  apply_cast<Output...>(transformed)};
    }
    catch (std::invalid_argument& e)
    {
        auto error_text = boost::format("Can not find requested input parameters: %1%") % e.what();
        LOG(error) << error_text;
        return {error_text.str(), {}};
    }
    catch (...)
    {
        auto error_text =  "Some error while input parameters parsing";
        LOG(error) << error_text;
        return {error_text, {}};
    }

    return {};
}

template <typename... Output, typename... Parameters>
std::optional< std::tuple< std::optional< Output >...> >
get_from_url_not_required(const std::unordered_map<std::string, std::string>& request, Parameters... parameters)
{
    using namespace std;
    using namespace nlohmann;
    using namespace cameramanagercore::utility;

    static_assert(sizeof...(Parameters) == sizeof...(Output), "Size of output parameters must be same with Input");

    try
    {
        auto requests = make_tuple(make_tuple(parameters, request.find(parameters))...);
        auto transformed = tuple_transform(requests, [&](auto r)
        { return std::get<1>(r) != request.end() ? std::make_optional(std::get<1>(r)->second)
                                                 : std::optional<typename std::decay_t<decltype(request)>::mapped_type> {}; });

        return apply_optional_cast<Output...>(transformed);
    }
    catch (...)
    {
        LOG(error) << "Some error while not required input parameters parsing";
        return {};
    }

    return {};
}

}


#endif //CAMERAMANAGERCORE_INPUTPARAMETERS_HPP
