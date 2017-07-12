//
// Created by svakhreev on 29.12.16.
//

#ifndef CAMERAMANAGERCORE_SERIALIZER_HPP
#define CAMERAMANAGERCORE_SERIALIZER_HPP

#include <initializer_list>
#include <memory>
#include <functional>
#include <tuple>
#include <string>
#include <type_traits>

#include <sqlite_modern_cpp.h>
#include "../../utility/LoggerSettings.hpp"

namespace cameramanagercore::database::statements
{

template<typename DatabaseAdapterPtr, typename ObjectTypePtr>
struct GetImpl;

template<typename DatabaseAdapterPtr, typename ObjectTypePtr>
struct AddImpl;

template<typename DatabaseAdapterPtr, typename ObjectTypePtr>
struct ChangeImpl;

template<typename DatabaseAdapterPtr, typename ObjectTypePtr>
struct RemoveImpl;

template<typename DatabaseAdapterPtr, typename ObjectTypePtr>
struct CreateTableImpl;

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto Get(DatabaseAdapterPtr&& adapter, ObjectTypePtr&&)
{
    return GetImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto Get(DatabaseAdapterPtr&& adapter)
{
    return GetImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto Add(DatabaseAdapterPtr&& adapter, std::initializer_list<ObjectTypePtr>&& objects)
{
    return AddImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter,
                                                        objects);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto Change(DatabaseAdapterPtr&& adapter, std::initializer_list<ObjectTypePtr> objects)
{
    return ChangeImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter,
                                                           objects);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto Remove(DatabaseAdapterPtr&& adapter, std::initializer_list<ObjectTypePtr>&& objects)
{
    return RemoveImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter,
                                                           objects);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto CreateTable(DatabaseAdapterPtr&& adapter)
{
    return CreateTableImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter);
}

template<typename ObjectTypePtr, typename DatabaseAdapterPtr>
auto CreateTable(DatabaseAdapterPtr&& adapter, ObjectTypePtr&&)
{
    return CreateTableImpl<DatabaseAdapterPtr, ObjectTypePtr>{}(adapter);
}

template<typename OnSuccess, typename OnError>
struct _ImplSqliteStatus;
template<typename>
struct _ImplOnSuccess;
template<typename>
struct _ImplOnError;
template<typename ReturnType>
struct _ImplSqliteExceptionsHandlerWithData;
template<typename>
struct _ImplSqliteExceptionsHandler;
template<typename>
struct _ImplCheck;

template<typename T>
using ErrorHandlerWithData = _ImplSqliteExceptionsHandlerWithData<T>;
using ErrorHandler = _ImplSqliteExceptionsHandler<int>;
using OnSuccess = _ImplOnSuccess<int>;
using OnError = _ImplOnError<int>;
using SqliteStatus = _ImplSqliteStatus<OnSuccess, OnError>;
using Check = _ImplCheck<int>;

template<typename>
struct _ImplOnSuccess
{
    std::function<void(SqliteStatus&)> handler = [](auto&) {};
};

template<typename>
struct _ImplOnError
{
    std::function<void(SqliteStatus&)> handler = [](auto&) {};
};

template<typename OnSuccess_, typename OnError_>
struct _ImplSqliteStatus
{
    bool is_error = false;
    std::string error_message = "";

    template<typename T>
    friend SqliteStatus  operator | (SqliteStatus status, _ImplOnSuccess<T> handler) { status.success = handler; return status; }

    template<typename T>
    friend SqliteStatus  operator | (SqliteStatus status, _ImplOnError<T> handler)   { status.error = handler; return status; }

    template<typename T>
    friend void  operator | (SqliteStatus status, _ImplCheck<T>) { status.is_error ? status.error.handler(status)
                                                                                   : status.success.handler(status); }

    OnSuccess_ success = { };
    OnError_ error = { };
};


template<typename ReturnType>
struct _ImplSqliteExceptionsHandlerWithData
{
    _ImplSqliteExceptionsHandlerWithData(std::function<ReturnType()> handler)
    {
        try
        {
            return_data = handler();
        }
        catch (sqlite::sqlite_exception& exception)
        {
            status = { true, exception.what() };
        }
    }

    std::tuple<SqliteStatus, ReturnType> result() { return std::make_tuple(status, return_data); };

    SqliteStatus status = {};
    ReturnType return_data = {};
};

template<typename>
struct _ImplSqliteExceptionsHandler
{
    _ImplSqliteExceptionsHandler(std::function<void()> handler)
    {
        try
        {
            handler();
        }
        catch (sqlite::sqlite_exception& exception)
        {
            status = { true, exception.what() };
        }
    }

    SqliteStatus result() { return status; }

    SqliteStatus status = {};
};

template<typename>
struct _ImplCheck {};

}

#endif //CAMERAMANAGERCORE_SERIALIZER_HPP
