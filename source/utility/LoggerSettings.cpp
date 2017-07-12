//
// Created by svakhreev on 30.03.17.
//

#include "LoggerSettings.hpp"

namespace cameramanagercore::utility
{

void coloring_formatter(const logging::record_view& rec,
                        logging::formatting_ostream& stream)
{
    auto severity = rec[logging::trivial::severity];
    if (severity)
    {
        switch (severity.get())
        {
            case logging::trivial::info:
                stream << "\033[1;94m";
                break;
            case logging::trivial::warning:
                stream << "\033[1;33m";
                break;
            case logging::trivial::error:
            case logging::trivial::fatal:
                stream << "\033[1;31m";
                break;
            default:
                break;
        }
    }

    logging::formatter f = expr::stream << expr::format_date_time<boost::posix_time::ptime>
        ("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");

    stream << logging::extract<uint32_t>("LineID", rec) << ": [";
    f(rec, stream);
    stream << "] [" << rec[logging::trivial::severity] << "]:\t"
           << "\033[0m"
           << rec[expr::smessage];
}

}
