/*
 * @Author: INotFound
 * @Date: 2021-01-06 10:01:06
 * @LastEditTime: 2021-01-08 14:06:59
 */
#pragma once
#include <pqxx/pqxx>
#include <RapidJson/writer.h>
#include <RapidJson/document.h>
#include <RapidJson/stringbuffer.h>
#include <Magic/DataBase/ConnectionPool.h>
#include <Magic/NetWork/Http/HttpServlet.h>

namespace WebServices {
    using namespace Magic::NetWork;
    class MovieListServlet : public Http::IHttpServlet {
    public:
        MovieListServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool);
        uint32_t getFilterValue(rapidjson::Document& jsonDoc);
        bool handle(const Safe <Http::HttpRequest> &request, const Safe <Http::HttpResponse> &response) override;

    private:
        Safe<Magic::DataBase::ConnectionPool<pqxx::connection>> m_ConnectionPool;
    };
}