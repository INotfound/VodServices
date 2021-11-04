#include "Servlet/MovieMenuServlet.h"
namespace WebServices {
    MovieMenuServlet::MovieMenuServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool)
        :Http::IHttpServlet("/getMovieMenu")
        ,m_ConnectionPool(connectionPool){
        m_ConnectionPool->initialize([](const Safe<pqxx::connection>& conn){
            conn->prepare("getMenuByType","select * from type");
            conn->prepare("getMenuByOrigin","select * from origin");
        });
    }

    bool MovieMenuServlet::handle(const Safe <Http::HttpRequest> &request, const Safe <Http::HttpResponse> &response){
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
        pqxx::result typeListResult;
        pqxx::result originListResult;

        try{
            auto conn = m_ConnectionPool->getConnection();
            if(conn){
                pqxx::work W(*(*conn));
                typeListResult = W.exec_prepared("getMenuByType");
                originListResult = W.exec_prepared("getMenuByOrigin");
            }
        }catch(std::exception& ec){
            MAGIC_WARN() << ec.what();
        }

        writer.StartObject();

        writer.Key("Data");
        writer.StartObject();

        writer.Key("TypeList");
        writer.StartArray();
        for(auto row : typeListResult){
            writer.StartObject();
            writer.Key("Id");
            writer.Int(row[0].as<int>());
            writer.Key("Name");
            writer.String(row[1].c_str());
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("OriginList");
        writer.StartArray();
        for(auto row : originListResult){
            writer.StartObject();
            writer.Key("Id");
            writer.Int(row[0].as<int>());
            writer.Key("Name");
            writer.String(row[1].c_str());
            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();

        writer.Key("Error");
        writer.Int(0);
        writer.Key("Msg");
        writer.String("");

        writer.EndObject();
        response->setBody(jsonBuf.GetString());
        response->setContentType(Http::HttpContentType::APPLICATION_JSON);
        return true;
    }
}