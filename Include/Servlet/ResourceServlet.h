/*
 * @Author: INotFound
 * @Date: 2021-01-06 10:01:06
 * @LastEditTime: 2021-01-08 14:06:59
 */
#pragma once
#include <vector>
#include <Magic/Magic>
#include <Magic/NetWork/Http/HttpServlet.h>

namespace WebServices{
    using namespace Magic::NetWork;
    class ResourceServlet :public Http::IHttpServlet{
    public:
        ResourceServlet(const Safe<Magic::Config>& configuration);
        bool handle(const Safe<Http::HttpRequest>& request,const Safe<Http::HttpResponse>& response) override;
    private:
        std::string m_Directory;
        std::vector<std::string> m_ImageFilter;
        std::vector<std::string> m_VideoFilter;
    };
}
