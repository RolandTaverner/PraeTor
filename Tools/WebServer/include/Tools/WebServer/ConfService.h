#ifndef WEBSERVER_INCLUDE_TOOLS_WEBSERVER_CONFSERVICE_H_
#define WEBSERVER_INCLUDE_TOOLS_WEBSERVER_CONFSERVICE_H_

// C++
#include <string>
#include <utility>

// BOOST
#include <boost/function.hpp>

#include "Tools/WebServer/IWebService.h"

namespace Tools
{
namespace WebServer
{

class ConfService : public Tools::WebServer::IWebService
{
public:
    explicit ConfService(boost::function<std::pair<std::string,std::string>()> confCallback);
    virtual ~ConfService();

    // IWebService overloads
    virtual void operator()(ConnectionContextPtr contextPtr);
    virtual void start(void);
    virtual void stop(void);

private:
    boost::function<std::pair<std::string,std::string>()> m_confCallback;
};

} /* namespace WebServer */
} /* namespace Tools */

#endif /* WEBSERVER_INCLUDE_TOOLS_WEBSERVER_CONFSERVICE_H_ */
