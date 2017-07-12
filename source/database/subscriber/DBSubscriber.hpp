//
// Created by svakhreev on 16.01.17.
//

#ifndef CAMERAMANAGERCORE_DBSUBSCRIBER_HPP
#define CAMERAMANAGERCORE_DBSUBSCRIBER_HPP

#include <memory>
#include <iostream>
#include <sstream>
#include <tuple>

#include "../../utility/LoggerSettings.hpp"
#include "../../signals/SignalContext.hpp"
#include "../../database/adapters/SqliteAdapter.hpp"
#include "../../database/statements/SqliteUserStatement.hpp"
#include "../../database/statements/SqliteCameraStatement.hpp"

namespace cameramanagercore::database::subscriber
{

using namespace cameramanagercore::database::statements;
using namespace cameramanagercore::signals;


template<typename SignalContext, typename Adapter, typename... Types>
struct DBSubscriber
{
    template<typename T, typename DBSubscriberContext>
    struct Callable
    {
        constexpr void operator()(SignalData<T> d)
        {
            context.ProcessSignal(d);
        }
        DBSubscriberContext& context;
    };

    using SignalContextPtr = std::shared_ptr<SignalContext>;

    DBSubscriber(SignalContextPtr signal_context, std::shared_ptr<Adapter> adapter)
        : _signal_context(signal_context),
          _db_adapter(adapter)
    {
//        _processors = std::make_tuple([this](SignalData<Types> d) { this->ProcessSignal(d); }...);
        _processors = std::make_tuple(Callable<Types, DBSubscriber<SignalContext, Adapter, Types...>>{ *this }...);
        (_signal_context->template AddSubscriber<Types>(std::get<std::function<void(SignalData<Types>)>>(_processors)), ...);
    }

    ~DBSubscriber()
    {
        (_signal_context->template RemoveSubscriber<Types>(std::get<std::function<void(SignalData<Types>)>>(_processors)), ...);
    }

private:
    template<typename T>
    void ProcessSignal(SignalData<T> data)
    {
        switch (data.status)
        {
            case DataStatus::Changed:
                LOG(info) << "Database CHANGE signal. Executing request now";
                Change(_db_adapter, { data.data })
                    | OnSuccess { [] (auto& status) { LOG(info) << "Successful CHANGE request"; } }
                    | OnError { [] (auto& status) { LOG(error) << "Error in CHANGE request: " + status.error_message; } }
                    | Check {};
                break;
            case DataStatus::Added:
                LOG(info) << "Database ADD signal. Executing request now";
                Add(_db_adapter,    { data.data })
                    | OnSuccess { [] (auto& status) { LOG(info) << "Successful ADD request"; } }
                    | OnError { [] (auto& status) { LOG(error) << "Error in ADD request: " + status.error_message; } }
                    | Check {};
                break;
            case DataStatus::Removed:
                LOG(info) << "Database REMOVE signal. Executing request now";
                Remove(_db_adapter, { data.data })
                    | OnSuccess { [] (auto& status) { LOG(info) << "Successful REMOVE request"; } }
                    | OnError { [] (auto& status) { LOG(error) << "Error in REMOVE request: " + status.error_message; } }
                    | Check {};
                break;
            default: break;
        }
    }

    SignalContextPtr _signal_context;
    std::shared_ptr<Adapter> _db_adapter;
    std::tuple<std::function<void(SignalData<Types>)>...> _processors;
};

}

#endif //CAMERAMANAGERCORE_DBSUBSCRIBER_HPP
