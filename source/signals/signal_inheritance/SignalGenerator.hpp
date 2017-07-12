//
// Created by svakhreev on 10.01.17.
//

#ifndef CAMERAMANAGERCORE_SIGNAL_GENERATOR_HPP
#define CAMERAMANAGERCORE_SIGNAL_GENERATOR_HPP

#include <memory>
#include <wigwag/signal.hpp>
#include <rttr/registration>
#include <boost/format.hpp>

#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::signals
{

enum class DataStatus
{
    Added, Changed, Removed
};

template<typename T>
struct SignalData
{
    using type = T;

    DataStatus status;
    std::shared_ptr<T> data;
};

template<typename T>
struct SignalGenerator : std::enable_shared_from_this<SignalGenerator<T>>
{
    using Data = SignalData<T>;

    wigwag::signal<void(Data)> s;

    void GenerateSignal(Data signal_data)
    {
        LOG(info) << boost::format("Generate signal: [%1%]: %2%")
                % rttr::variant { signal_data.status }.to_string()
                % *signal_data.data;
        s(signal_data);
    }

    SignalGenerator(const SignalGenerator<T>&) = delete;
    SignalGenerator& operator=(const SignalGenerator<T>&) = delete;

    SignalGenerator(SignalGenerator<T>&&) = default;
    SignalGenerator& operator=(SignalGenerator<T>&&) = default;

    static std::shared_ptr<SignalGenerator<T>> Create() { return std::shared_ptr<SignalGenerator<T>>(new SignalGenerator()); }

private:
    SignalGenerator() = default;
};

template<typename T>
SignalData<T> create_signal_data(DataStatus status, std::shared_ptr<T> data)
{
    return { status, data };
}

}


#endif //CAMERAMANAGERCORE_SIGNAL_GENERATOR_HPP
