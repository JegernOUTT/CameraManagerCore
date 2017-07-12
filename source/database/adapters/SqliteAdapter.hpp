//
// Created by svakhreev on 27.12.16.
//

#ifndef CAMERAMANAGERCORE_SQLITEADAPTER_HPP
#define CAMERAMANAGERCORE_SQLITEADAPTER_HPP

#include <memory>
#include <algorithm>
#include <list>

#include <boost/format.hpp>
#include <boost/circular_buffer.hpp>

#include <sqlite_modern_cpp.h>

#include "./Adapter.hpp"
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::database::adapters
{

template<>
struct Adapter<sqlite::database>
{
    using DbPtr = std::shared_ptr<sqlite::database>;
    using BufferPtr = std::shared_ptr<boost::circular_buffer<std::string>>;

    Adapter(DbPtr db)
        : _db(db),
          _command_log(std::make_shared<boost::circular_buffer<std::string>>(1'000))
    {
    }

    Adapter& operator << (std::string c)
    {
        LOG(debug) << c;
        _current_command = c;
        _command_log->push_back(c);
        return *this;
    };

    Adapter& operator << (AddBlob b)
    {
        _blobs.emplace_back(b);
        return *this;
    };

    template < typename Visitor >
    Adapter& operator >> (Visitor&& f)
    {
        LOG(info) << "Executing request from adapter";

        auto statement = (*_db) << _current_command;
        if (_blobs.size() > 0)
            statement << _blobs.front().blob;
        statement >> std::forward<Visitor>(f);

        _current_command.clear();
        _blobs.clear();

        return *this;
    }

    Adapter& operator >> (ExecuteEmpty)
    {
        LOG(info) << "Executing request from adapter";

        auto statement = (*_db) << _current_command;
        if (_blobs.size() > 0)
            statement << _blobs.front().blob;

        _current_command.clear();
        _blobs.clear();

        return *this;
    };

    auto GetDb() const { return _db; }
    auto GetCommandLog() const { return _command_log; }

private:
    DbPtr _db;
    BufferPtr _command_log;
    std::string _current_command;
    std::list<AddBlob> _blobs;
};

}
#endif //CAMERAMANAGERCORE_SQLITEADAPTER_HPP
