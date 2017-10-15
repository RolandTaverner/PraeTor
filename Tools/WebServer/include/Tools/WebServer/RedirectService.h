#pragma once

#include <map>
#include <string>

#include "Tools/WebServer/IWebService.h"

namespace Tools
{
namespace WebServer
{

class RedirectService : public Tools::WebServer::IWebService
{
public:
    RedirectService(const std::string &from, const std::string &to, int httpStatus);
    
    ~RedirectService();

    virtual void operator()(Tools::WebServer::ConnectionContextPtr contextPtr);
    virtual void start(void);
    virtual void stop(void);

private:
    const std::string &getStatusMessage(unsigned statusCode) const;

    const std::string m_from;
    const std::string m_to;
    const int m_httpStatus;
    std::map<unsigned, std::string> m_httpStatusMessage;
};

} /* namespace WebServer */
} /* namespace Tools */

