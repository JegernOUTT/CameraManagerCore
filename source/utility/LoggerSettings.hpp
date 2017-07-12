//
// Created by svakhreev on 01.03.17.
//
#ifndef CAMERAMANAGERCORE_LOGGERSETTINGS_HPP
#define CAMERAMANAGERCORE_LOGGERSETTINGS_HPP

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

#ifndef LOG
#define LOG(lvl) BOOST_LOG_TRIVIAL(lvl)
#endif

/*
 *  LOG(trace)   << "Trace";
    LOG(debug)   << "Debug";
    LOG(info)    << "Informational";
    LOG(warning) << "Warning";
    LOG(error)   << "Error";
    LOG(fatal)   << "Fatal";
 *
 * */

namespace cameramanagercore::utility
{

enum class LoggerOutput
{
    File, Stdout, FileStdout
};

void coloring_formatter(const logging::record_view& rec,
                        logging::formatting_ostream& strm);

struct LoggerSettings
{
    LoggerSettings(LoggerOutput type)
    {
        switch(type)
        {
            case LoggerOutput::File: init_file(); break;
            case LoggerOutput::Stdout: init_stdout(); break;
            case LoggerOutput::FileStdout: init_file(); init_stdout(); break;
        }
        logging::add_common_attributes();
    }

private:
    void init_file()
    {
        logging::add_file_log
            (
                keywords::file_name = "sample_%N.log",
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                keywords::auto_flush = true,
                keywords::format = (expr::stream
                    << expr::attr<uint32_t>("LineID")
                    << ": [" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                                 "%Y-%m-%d %H:%M:%S.%f")
                    << "] [" << logging::trivial::severity
                    << "] "  << expr::smessage)
            );

        logging::core::get()->set_filter
            (
                logging::trivial::severity >= logging::trivial::debug
            );
    };

    void init_stdout()
    {
        auto sink = boost::make_shared<sinks::synchronous_sink<sinks::text_ostream_backend>>();
        sink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::cout,
                                                                           [](std::ostream*) {}));
        sink->locked_backend()->auto_flush(true);
        sink->set_formatter
            (
                expr::stream
                    << expr::attr<uint32_t>("LineID")
                    << ": [" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp",
                                                                                 "%Y-%m-%d %H:%M:%S.%f")
                    << "] [" << logging::trivial::severity
                    << "] "  << expr::smessage
            );
        sink->set_formatter(&coloring_formatter);
        logging::core::get()->add_sink(sink);
    };
};

}

#endif //CAMERAMANAGERCORE_LOGGERSETTINGS_HPP