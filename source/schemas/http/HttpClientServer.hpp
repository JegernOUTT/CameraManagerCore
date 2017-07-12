//
// Created by svakhreev on 24.03.17.
//

#ifndef CAMERAMANAGERCORE_HTTPCLIENTSERVER_HPP
#define CAMERAMANAGERCORE_HTTPCLIENTSERVER_HPP

#include <tuple>
#include <iostream>
#include <memory>
#include <simple-web-server/server_http.hpp>

namespace cameramanagercore::schemas
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using Response = typename HttpServer::Response;
using Request = typename HttpServer::Request;

template < typename Contexts, template <typename> class... Schemas>
struct HttpClientServer
{
    HttpClientServer(Contexts contexts, int port, int thread_count)
        : _server(std::make_shared<HttpServer>(port, thread_count)),
          _contexts(contexts),
          _schemas(make_tuple(Schemas<Contexts>{ }...))
    {
    }

    void Start()
    {
        std::apply([&](Schemas<Contexts>&... s) { ((s.CreateSchemas(_contexts, _server)), ...); }, _schemas);
        _server->default_resource["GET"] = [] (std::shared_ptr<Response> response, std::shared_ptr<Request> request)
        {

        };
        _server->start();
    }

    void Stop()
    {
        _server->stop();
    }

private:
    std::shared_ptr<HttpServer> _server;
    Contexts _contexts;
    std::tuple<Schemas<Contexts>...> _schemas;
};

}

#endif //CAMERAMANAGERCORE_HTTPCLIENTSERVER_HPP
