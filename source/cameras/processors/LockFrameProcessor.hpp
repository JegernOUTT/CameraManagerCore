//
// Created by svakhreev on 14.03.17.
//

#ifndef CAMERAMANAGERCORE_LOCKFRAMEPROCESSOR_HPP
#define CAMERAMANAGERCORE_LOCKFRAMEPROCESSOR_HPP

#include <memory>

#include "../../utility/LoggerSettings.hpp"
#include "../../signals/SignalContext.hpp"

namespace cameramanagercore::cameras::processors
{

using namespace cameramanagercore::signals;

template <typename SignalContext, typename CameraContext, typename ModelContext, typename EventContext, typename ThreadPool>
class LockFrameProcessor
{
    using CameraContextPtr = std::shared_ptr<CameraContext>;
    using ModelContextPtr  = std::shared_ptr<ModelContext>;
    using ThreadPoolPtr    = std::shared_ptr<ThreadPool>;
    using SignalContextPtr = std::shared_ptr<SignalContext>;
    using EventContextPtr  = std::shared_ptr<EventContext>;

public:
    LockFrameProcessor(SignalContextPtr signal_context,
                       CameraContextPtr camera_context,
                       ModelContextPtr model_context,
                       EventContextPtr event_context,
                       ThreadPoolPtr thread_pool)
        : _camera_context(camera_context)
    {
    }

    void operator()()
    {
        LOG(info) << "Lock frames generation for all cameras";
        _camera_context->GenerateLockFramesSignals(DataStatus::Changed);
    }

private:
    CameraContextPtr _camera_context;
};

}


#endif //CAMERAMANAGERCORE_LOCKFRAMEPROCESSOR_HPP
