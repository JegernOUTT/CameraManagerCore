//
// Created by svakhreev on 06.03.17.
//

#ifndef CAMERAMANAGERCORE_CONTEXTMISC_HPP
#define CAMERAMANAGERCORE_CONTEXTMISC_HPP

#include <functional>
#include <memory>

#include "../User.hpp"
#include "../Event.hpp"

namespace cameramanagercore::model
{
enum class OperationStatus { Ok, SameId, SameBody, NotFound, UnknownError };
}

namespace std
{
template<> struct hash< weak_ptr<cameramanagercore::model::User> >
{
    size_t operator()(const weak_ptr<cameramanagercore::model::User>& user) const
    {
        if (!user.expired())
            return hash<string>{}(user.lock()->name)
                   ^ hash<string>{}(user.lock()->password)
                   ^ hash<int>{}(user.lock()->id);
        return size_t {};
    }

    size_t operator()(const shared_ptr<cameramanagercore::model::Event>& event) const
    {
        if (event)
            return hash<string>{}(event->hash)
                   ^ hash<string>{}(event->name);
        return size_t {};
    }
};
}

#endif //CAMERAMANAGERCORE_CONTEXTMISC_HPP
