#include "Servlet/AddMovieServlet.h"
namespace WebServices {
    AddMovieServlet::AddMovieServlet(const Safe<Magic::DataBase::ConnectionPool<pqxx::connection>>& connectionPool)
        :Http::IHttpServlet("/AddMovie")
        ,m_ConnectionPool(connectionPool){
        m_ConnectionPool->initialize([](const Safe<pqxx::connection>& conn){
            conn->prepare("addMovie","insert into Movie(Name,Score,Details,Views,Director,Starring,Type,Origin,SpellName,AddedTime,ReleaseTime) values ($1,$2,$3,$4,$5,$6,$7,$8,$9,current_date,$10) returning Id");
        });
    }

    bool AddMovieServlet::handle(const Safe <Http::HttpRequest> &request, const Safe <Http::HttpResponse> &response){
        bool error = false;
        pqxx::result result;
        std::string errorString;
        rapidjson::Document jsonDoc;
        rapidjson::StringBuffer jsonBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(jsonBuf);

        if(!jsonDoc.Parse(request->getBody().c_str()).HasParseError()){

            if(jsonDoc.IsObject()
                && jsonDoc.HasMember("Name")
                && jsonDoc.HasMember("Score")
                && jsonDoc.HasMember("Details")
                && jsonDoc.HasMember("Views")
                && jsonDoc.HasMember("Director")
                && jsonDoc.HasMember("Starring")
                && jsonDoc.HasMember("Type")
                && jsonDoc.HasMember("Origin")
                && jsonDoc.HasMember("SpellName")
                && jsonDoc.HasMember("ReleaseTime")){

                if(jsonDoc["Name"].IsString()
                    && (jsonDoc["Score"].IsDouble() || jsonDoc["Score"].IsInt())
                    && jsonDoc["Details"].IsString()
                    && jsonDoc["Views"].IsInt64()
                    && jsonDoc["Director"].IsString()
                    && jsonDoc["Starring"].IsString()
                    && jsonDoc["Type"].IsString()
                    && jsonDoc["Origin"].IsInt()
                    && jsonDoc["SpellName"].IsString()
                    && jsonDoc["ReleaseTime"].IsString()){

                    int origin = jsonDoc["Origin"].GetInt();
                    double score = jsonDoc["Score"].GetDouble();
                    int64_t views = jsonDoc["Views"].GetInt64();

                    std::string name(jsonDoc["Name"].GetString());
                    std::string details(jsonDoc["Details"].GetString());
                    std::string director(jsonDoc["Director"].GetString());
                    std::string starring(jsonDoc["Starring"].GetString());
                    std::string type(jsonDoc["Type"].GetString());
                    std::string spellName(jsonDoc["SpellName"].GetString());
                    std::string releaseTime(jsonDoc["ReleaseTime"].GetString());

                    if(origin > 0
                        && score >= 0.0
                        && views >= 0
                        && !name.empty()
                        && !details.empty()
                        && !director.empty()
                        && !starring.empty()
                        && !type.empty()
                        && !spellName.empty()
                        && !releaseTime.empty()){

                        try{
                            auto conn = m_ConnectionPool->getConnection();
                            if(conn){
                                pqxx::work W(*(*conn));
                                result = W.exec_prepared("addMovie",name,score,details,views,director,starring,type,origin,spellName,releaseTime);
                                W.commit();
                            }
                        }catch(std::exception& ec){
                            MAGIC_WARN() << ec.what();
                        }

                    }
                }
            }
        }
        writer.StartObject();

        writer.Key("Data");
        writer.StartObject();
        if(result.size() > 0){
            writer.Key("Id");
            writer.Int64(result[0][0].as<int64_t>());
        }else{
            error = true;
            errorString = "添加失败";
        }
        writer.EndObject();

        writer.Key("Error");
        writer.Int(error ? 1 : 0);
        writer.Key("Msg");
        writer.String(error ? errorString.c_str() : "");

        writer.EndObject();
        response->setBody(jsonBuf.GetString());
        response->setContentType(Http::HttpContentType::APPLICATION_JSON);
        return true;
    }
}