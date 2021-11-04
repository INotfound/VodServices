#pragma once
#include <pqxx/pqxx>
#include <RapidJson/writer.h>
#include <RapidJson/stringbuffer.h>
#include <Magic/DataBase/ConnectionPool.h>
#include <Magic/NetWork/Http/HttpServlet.h>

namespace WebServices {
    using namespace Magic::NetWork;
    class MovieMenuServlet : public Http::IHttpServlet{
    public:
        MovieMenuServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool);
        bool handle(const Safe <Http::HttpRequest> &request, const Safe <Http::HttpResponse> &response) override;
    private:
        Safe<Magic::DataBase::ConnectionPool<pqxx::connection>> m_ConnectionPool;
    };
}