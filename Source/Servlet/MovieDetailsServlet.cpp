#include "Servlet/MovieDetailsServlet.h"
namespace WebServices {
    MovieDetailsServlet::MovieDetailsServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool)
        :Http::IHttpServlet("/getMovieDetails")
        ,m_ConnectionPool(connectionPool){
        m_ConnectionPool->initialize([](const Safe<pqxx::connection>& conn){
            conn->prepare("getMovieDetails","select m.Id,m.Name,m.Score,m.Details,m.Starring,o.Name,array_to_string(array_agg(t.Name),'/'),m.ReleaseTime from Movie as m join Type as t on t.Id = ANY(m.Type) join Origin as o on o.Id = m.Origin where m.Id = $1 GROUP BY m.Id,m.Name,m.Score,m.Details,m.Starring,o.Name,m.ReleaseTime");
        });
    }

    bool MovieDetailsServlet::handle(const Safe <Http::HttpRequest> &request, const Safe <Http::HttpResponse> &response){
        pqxx::result result;
        rapidjson::StringBuffer jsonBuf;
        const std::string& id = request->getParam("Id");
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);
        try{
            auto conn = m_ConnectionPool->getConnection();
            if(conn){
                pqxx::work W(*(*conn));
                result = W.exec_prepared("getMovieDetails",Magic::StringAs<int>(id));
            }
        }catch(std::exception& ec){
            MAGIC_WARN() << ec.what();
        }

        writer.StartObject();

        writer.Key("Data");
        writer.StartObject();

        if(result.size() != 0){
            writer.Key("Id");
            writer.Int64(result[0][0].as<int64_t>());
            writer.Key("Name");
            writer.String(result[0][1].c_str());
            writer.Key("Score");
            writer.Double(result[0][2].as<double>());
            writer.Key("Details");
            writer.String(result[0][3].c_str());
            writer.Key("Starring");
            writer.String(result[0][4].c_str());
            writer.Key("Origin");
            writer.String(result[0][5].c_str());
            writer.Key("Type");
            writer.String(result[0][6].c_str());
            writer.Key("ReleaseTime");
            writer.String(result[0][7].c_str());
        }

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