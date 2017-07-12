//
// Created by svakhreev on 24.03.17.
//

#ifndef CAMERAMANAGERCORE_CONTEXTSWRAPPER_HPP
#define CAMERAMANAGERCORE_CONTEXTSWRAPPER_HPP

#include <memory>
#include <tuple>
#include <type_traits>

namespace cameramanagercore::schemas
{

template <typename... Contexts>
struct ContextsWrapper
{
    std::tuple<std::shared_ptr<Contexts>...> contexts;

    template<typename Context>
    std::shared_ptr<Context> Get()
    {
        return std::get<std::shared_ptr<Context>>(contexts);
    }
};

template <typename... Contexts>
auto make_contexts_wrapper(std::shared_ptr<Contexts>... contexts)
{
    return ContextsWrapper<Contexts...> { std::make_tuple(contexts...) };
}

}

#endif //CAMERAMANAGERCORE_CONTEXTSWRAPPER_HPP
