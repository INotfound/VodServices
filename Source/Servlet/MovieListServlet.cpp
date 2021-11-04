#include <limits>
#include <cstring>
#include <Magic/Utilty/Logger.h>
#include "Servlet/MovieListServlet.h"

namespace WebServices{

    static const uint32_t g_Type = 1000;
    static const uint32_t g_Date = 2000;
    static const uint32_t g_Search = 3000;
    static const uint32_t g_Origin = 4000;

    MovieListServlet::MovieListServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool)
        :Http::IHttpServlet("/getMovieList")
        ,m_ConnectionPool(connectionPool){
        m_ConnectionPool->initialize([](const Safe<pqxx::connection>& conn){
            /*
             * By:["Search"]
             * "Search": "ZL",
             * "Position": 1,
             * "Sort": 0
             * */
            conn->prepare("getMovie","select Id,Name,Score from movie order by (addedtime,views) desc limit $1 offset $2");
            conn->prepare("getMovieBySearch","select Id,Name,Score from movie where SpellName like $1 limit $2 offset $3");
            conn->prepare("getMovieByType","select Id,Name,Score from movie where $1 = ANY(type) order by (addedtime,views) desc limit $2 offset $3");
            conn->prepare("getMovieByOrigin","select Id,Name,Score from movie where Origin = $1  order by (addedtime,views) desc limit $2 offset $3");
            conn->prepare("getMovieByDate","select Id,Name,Score from movie where ReleaseTime >= $1::date and ReleaseTime <= $2::date limit $3 offset $4");

            conn->prepare("getMoiveByTypeAndOrigin","select Id,Name,Score from movie where $1 = ANY(type) and $2 = Origin order by (addedtime,views) desc limit $3 offset $4");
            conn->prepare("getMoiveByOriginAndDate","select Id,Name,Score from movie where $1 = Origin and ReleaseTime >= $2::date and ReleaseTime < $3::date order by (addedtime,views) desc limit $4 offset $5");
            conn->prepare("getMoiveByTypeAndDate","select Id,Name,Score from movie where $1 = ANY(type) and ReleaseTime >= $2::date and ReleaseTime < $3::date order by (addedtime,views) desc limit $4 offset $5");
            conn->prepare("getMoiveByTypeAndOriginAndDate","select Id,Name,Score from movie where $1 = ANY(type) and $2 = Origin and ReleaseTime >= $3::date and ReleaseTime < $4::date order by (addedtime,views) desc limit $5 offset $6");
        });
    }

    uint32_t MovieListServlet::getFilterValue(rapidjson::Document& jsonDoc){
        uint32_t filter = 0;
        if(jsonDoc.IsObject() && jsonDoc.HasMember("By")){
            auto jsonArray = jsonDoc["By"].GetArray();
            for(auto& v : jsonArray){
                if(std::strcmp("Type",v.GetString()) == 0){
                    filter |= g_Type;
                    continue;;
                }
                if(std::strcmp("Date",v.GetString()) == 0){
                    filter |= g_Date;
                    continue;
                }
                if(std::strcmp("Search",v.GetString()) == 0){
                    filter |= g_Search;
                    continue;;
                }
                if(std::strcmp("Origin",v.GetString()) == 0){
                    filter |= g_Origin;
                    continue;;
                }
            }
        }
        return filter;
    }

    bool MovieListServlet::handle(const Safe<Http::HttpRequest>& request,const Safe<Http::HttpResponse>& response){
        bool error = false;
        std::string errorString;
        rapidjson::Document jsonDoc;
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
        if(jsonDoc.Parse(request->getBody().c_str()).HasParseError()){
            error = true;
            errorString += "Json 解析错误;";
        }
        writer.StartObject();

        if(jsonDoc.IsObject()
            && jsonDoc.HasMember("By")
            && jsonDoc["By"].IsArray()
            && jsonDoc.HasMember("Position")
            && jsonDoc["Position"].IsInt()){
            pqxx::result result;
            uint32_t filterSize = 120;
            uint32_t filter = this->getFilterValue(jsonDoc);
            auto position = static_cast<uint32_t>(jsonDoc["Position"].GetInt());

            try {
                switch(filter){
                    case 0:{
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMovie", filterSize, position);
                        }else{
                            MAGIC_WARN() << "数据库连接池中获取超时!";
                        }
                    }
                        break;
                    case g_Type:{
                        uint32_t type = 0;
                        if(!jsonDoc["Type"].IsInt()){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        type = jsonDoc["Type"].GetInt();
                        if(type <= 0){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMovieByType", type, filterSize, position);
                        }else{
                            MAGIC_WARN() << "数据库连接池中获取超时!";
                        }
                    }
                        break;
                    case g_Date:{
                        if(!jsonDoc["Date"].IsString()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string field(jsonDoc["Date"].GetString());
                        if(field.empty()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string begin = field + "-1-1";
                        std::string end = field + "-12-31";
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMovieByDate",begin.c_str(),end.c_str(),filterSize,position);
                        }
                    }
                        break;
                    case g_Search:{
                        if(!jsonDoc["Search"].IsString()){
                            error = true;
                            errorString += "Search Field Error\n";
                            break;
                        }
                        std::string field(jsonDoc["Search"].GetString());
                        if(field.empty()){
                            error = true;
                            errorString += "Search Field Error\n";
                            break;
                        }
                        field += '%';
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMovieBySearch", field.c_str(), filterSize, position);
                        }
                    }
                        break;
                    case g_Origin:{
                        uint32_t origin = 0;
                        if(!jsonDoc["Origin"].IsInt()){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        origin = jsonDoc["Origin"].GetInt();
                        if(origin <= 0){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMovieByOrigin",origin,filterSize,position);
                        }
                    }
                        break;
                    case g_Type | g_Date:{
                        uint32_t type = 0;
                        if(!jsonDoc["Type"].IsInt()){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        type = jsonDoc["Type"].GetInt();
                        if(type <= 0){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        if(!jsonDoc["Date"].IsString()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string field(jsonDoc["Date"].GetString());
                        if(field.empty()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string begin = field + "-1-1";
                        std::string end = field + "-12-31";
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMoiveByTypeAndDate",type,begin,end,filterSize,position);
                        }
                    }
                        break;
                    case g_Type | g_Origin:{
                        uint32_t type = 0;
                        uint32_t origin = 0;
                        if(!jsonDoc["Type"].IsInt()){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        type = jsonDoc["Type"].GetInt();
                        if(type <= 0){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        if(!jsonDoc["Origin"].IsInt()){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        origin = jsonDoc["Origin"].GetInt();
                        if(origin <= 0){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMoiveByTypeAndOrigin",type,origin,filterSize,position);
                        }
                    }
                        break;
                    case g_Date | g_Origin:{
                        uint32_t origin = 0;
                        if(!jsonDoc["Origin"].IsInt()){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        origin = jsonDoc["Origin"].GetInt();
                        if(origin <= 0){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        if(!jsonDoc["Date"].IsString()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string field(jsonDoc["Date"].GetString());
                        if(field.empty()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string begin = field + "-1-1";
                        std::string end = field + "-12-31";
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMoiveByOriginAndDate", origin, begin, end, filterSize,position);
                        }
                    }
                        break;
                    case g_Type | g_Date | g_Origin:{
                        uint32_t type = 0;
                        uint32_t origin = 0;
                        if(!jsonDoc["Type"].IsInt()){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        type = jsonDoc["Type"].GetInt();
                        if(type <= 0){
                            error = true;
                            errorString += "Type Field Error\n";
                            break;
                        }
                        if(!jsonDoc["Date"].IsString()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        std::string field(jsonDoc["Date"].GetString());
                        if(field.empty()){
                            error = true;
                            errorString += "Date Field Error\n";
                            break;
                        }
                        if(!jsonDoc["Origin"].IsInt()){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        origin = jsonDoc["Origin"].GetInt();
                        if(origin <= 0){
                            error = true;
                            errorString += "Origin Field Error\n";
                            break;
                        }
                        std::string begin = field + "-1-1";
                        std::string end = field + "-12-31";
                        auto conn = m_ConnectionPool->getConnection();
                        if(conn){
                            pqxx::work W(*(*conn));
                            result = W.exec_prepared("getMoiveByTypeAndOriginAndDate",type,origin,begin,end,filterSize,position);
                        }
                    }
                        break;
                    default:
                        break;
                }
            }catch(std::exception& ex) {
                error = true;
                errorString += ex.what();
                MAGIC_WARN() << "Filter: ["<< filter << "] " << errorString;
            }

            writer.Key("Data");
            writer.StartArray();
            for (auto row: result){
                writer.StartObject();
                writer.Key("Id");
                writer.Int(row[0].as<int>());
                writer.Key("Name");
                writer.String(row[1].c_str());
                writer.Key("Score");
                writer.Double(row[2].as<double>());
                writer.EndObject();
            }
            writer.EndArray();

        }else{
            error = true;
            errorString += "Json 缺少字段 [By];";
        }

        writer.Key("Error");
        writer.Int(error ? 1 : 0);
        writer.Key("Msg");
        writer.String(error ? errorString.c_str() : "");
        writer.EndObject();

        response->setBody(jsonBuf.GetString());
        response->setContentType(Http::HttpContentType::APPLICATION_JSON);
        jsonBuf.Clear();
        return true;
    }
}