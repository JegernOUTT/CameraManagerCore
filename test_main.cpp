//
// Created by svakhreev on 28.12.16.
//
#include <iostream>
#include <exception>
#include <signal.h>
#include <bandit/bandit.h>
#include <boost/stacktrace.hpp>
#include "source/utility/LoggerSettings.hpp"

void my_terminate_handler()
{
    std::cerr << "Terminate called:\n"
              << boost::stacktrace::stacktrace()
              << '\n';
    std::abort();
}

void my_signal_handler(int signum)
{
    ::signal(signum, SIG_DFL);
    boost::stacktrace::stacktrace bt;
    if (bt)
    {
        std::cerr << "Signal "
                  << signum << ", backtrace:\n"
                  << boost::stacktrace::stacktrace() << '\n';
    }
    _Exit(-1);
}

int main(int argc, char** argv)
{
    std::set_terminate(&my_terminate_handler);
    ::signal(SIGSEGV, &my_signal_handler);
    ::signal(SIGABRT, &my_signal_handler);

    cameramanagercore::utility::LoggerSettings
        logger_init(cameramanagercore::utility::LoggerOutput::File);

    return bandit::run(argc, argv);
}