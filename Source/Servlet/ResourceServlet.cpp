/*
 * @Author: INotFound
 * @Date: 2021-01-06 10:01:13
 * @LastEditTime: 2021-01-12 08:07:17
 */
#include <Magic/Utilty/Logger.h>
#include "Servlet/ResourceServlet.h"

namespace WebServices{
    static bool IsFileExists(const std::string& path){
        if (FILE *file = fopen(path.c_str(), "r")) {
            fclose(file);
            return true;
        } else {
            return false;
        }
    }

    ResourceServlet::ResourceServlet(const Safe<Magic::Config>& configuration)
        :Http::IHttpServlet("^/?(.*)$",Http::HttpServletType::Global)
        ,m_Directory(configuration->at<std::string>("WebServices.ResourceServlet.Directory","./"))
        ,m_ImageFilter({".jpg",".png",".jpeg"})
        ,m_VideoFilter({".avi",".mkv",".mp4",".flv",".rmvb"}){
    }

    bool ResourceServlet::handle(const Safe<Http::HttpRequest>& request,const Safe<Http::HttpResponse>& response){
        std::string path = request->getPath();
        std::string resource =  m_Directory + path;

        if(path.length() > 2 && path.at(1) == 'I'){
            for(auto& v : m_ImageFilter){
                std::string completePath = resource + v;
                if(IS_FILE(completePath.c_str()) == 0 && IsFileExists(completePath)){
                    response->setResource(completePath);
                    response->setContentType(Http::FileTypeToHttpContentType(completePath));
                    return true;
                }
            }
        }else if(path.length() > 2 && path.at(1) == 'V'){
            for(auto& v : m_VideoFilter){
                std::string completePath = resource + v;
                if(IS_FILE(completePath.c_str()) == 0 && IsFileExists(completePath)){
                    response->setResource(completePath);
                    response->setContentType(Http::FileTypeToHttpContentType(completePath));
                    return true;
                }
            }
        }

        return false;
    }
}