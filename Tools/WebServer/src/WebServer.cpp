// BOOST
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

// PION
#include <pion/scheduler.hpp>

#include "Tools/Logger/Logger.h"
#include "Tools/WebServer/IWebService.h"
#include "Tools/WebServer/PionWebServerCore.h"
#include "Tools/WebServer/WebServer.h"
#include "Tools/WebServer/Scheduler.h"
#include "Tools/WebServer/ServiceHandler.h"
#include "Tools/WebServer/StatService.h"
#include "Tools/WebServer/ConfService.h"

namespace Tools
{
namespace WebServer
{

// Заглушка для статистики
class StatStub : public IStat
{
public:
    virtual ~StatStub() {}

    virtual void registerParameter(const std::string &) {}
    virtual void set(const std::string &, const long) {}
    virtual void increment(const std::string &, const long) {}
    virtual void registerParametersProvider(const std::string &name,
    		                                const ParametersProvider &parametersProvider){};
    virtual void unregisterParametersProvider(const std::string &name){};
};

struct WebServer::WebServerImpl
{
    WebServerImpl(const boost::asio::ip::tcp::endpoint &endpoint,
         std::size_t threadCount,
         std::size_t timeout,
         std::size_t connectionLimit)
    {
        m_pionScheduler.set_num_threads(threadCount);

        m_pionWebServerCorePtr.reset(new Detail::PionWebServerCore(m_pionScheduler,
                                                                   endpoint,
                                                                   timeout,
                                                                   connectionLimit));
    }

    Detail::PionWebServerCorePtr m_pionWebServerCorePtr;
    pion::single_service_scheduler m_pionScheduler;
};

//--------------------------------------------------------------------------------------------------
WebServer::WebServer():
        m_activeRequestsCount(0),
        m_port(80u),
        m_httpThreadCount(8u),
        m_timeout(1u),
        m_connectionLimit(100u),
        m_workerThreadCount(16u),
        m_isRunning(false),
        m_enableStat(false),
        m_statPtr(new StatStub())
{
}

//--------------------------------------------------------------------------------------------------
WebServer::WebServer(const std::string &host,
                             const unsigned int port,
                             const std::size_t httpThreadCount,
                             const std::size_t timeout,
                             const std::size_t connectionLimit,
                             const std::size_t workerThreadCount):
        m_host(host),
        m_port(port),
        m_httpThreadCount(httpThreadCount),
        m_timeout(timeout),
        m_connectionLimit(connectionLimit),
        m_workerThreadCount(workerThreadCount),
        m_isRunning(false),
        m_enableStat(false),
        m_statPtr(new StatStub())
{
}

//--------------------------------------------------------------------------------------------------
WebServer::~WebServer()
{
    if (isRunning())
    {
        stop();
    }
}

//--------------------------------------------------------------------------------------------------
std::size_t WebServer::getConnectionLimit() const
{
    return m_connectionLimit;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setConnectionLimit(std::size_t connectionLimit)
{
    m_connectionLimit = connectionLimit;
}

//--------------------------------------------------------------------------------------------------
const std::string &WebServer::getHost() const
{
    return m_host;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setHost(const std::string& host)
{
    if (isRunning())
    {
        throw WebServerError("Can't modify parameters on running server");
    }
    m_host = host;
}

//--------------------------------------------------------------------------------------------------
bool WebServer::isRunning() const
{
    return m_isRunning;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setRunning(bool isRunning)
{
    m_isRunning = isRunning;
}

//--------------------------------------------------------------------------------------------------
unsigned int WebServer::getPort() const
{
    return m_port;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setPort(unsigned int port)
{
    if (isRunning())
    {
        throw WebServerError("Can't modify parameters on running server");
    }
    m_port = port;
}

//--------------------------------------------------------------------------------------------------
std::size_t WebServer::getHttpThreadCount() const
{
    return m_httpThreadCount;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setHttpThreadCount(std::size_t threadCount)
{
    if (isRunning())
    {
        throw WebServerError("Can't modify parameters on running server");
    }
    m_httpThreadCount = threadCount;
}

//--------------------------------------------------------------------------------------------------
std::size_t WebServer::getTimeout() const
{
    return m_timeout;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setTimeout(std::size_t timeout)
{
    if (isRunning())
    {
        throw WebServerError("Can't modify parameters on running server");
    }
    m_timeout = timeout;
}

//--------------------------------------------------------------------------------------------------
void WebServer::addService(const std::string &resource, IWebServicePtr servicePtr)
{
    if (isRunning())
    {
        throw WebServerError("WebServer already running");
    }

    if (m_services.find(resource) != m_services.end() || m_pluginServices.find(resource) != m_pluginServices.end())
    {
        throw WebServerError("Resource \"" + resource + "\" already in use");
    }

    m_services[resource] = servicePtr;
}

//--------------------------------------------------------------------------------------------------
void WebServer::addPluginService(const std::string &resource,
    PluginServicePtr servicePtr,
    const PluginServiceOptions &options)
{
    if (isRunning())
    {
        throw WebServerError("WebServer already running");
    }

    if (m_services.find(resource) != m_services.end() || m_pluginServices.find(resource) != m_pluginServices.end())
    {
        throw WebServerError("Resource \"" + resource + "\" already in use");
    }

    m_pluginServices[resource] = std::make_pair<>(servicePtr, options);
}

//--------------------------------------------------------------------------------------------------
void WebServer::start()
{
    if (isRunning())
    {
        throw WebServerError("WebServer already running");
    }

    if (m_services.empty() && m_pluginServices.empty())
    {
        throw WebServerError("No services added");
    }
    BOOST_ASSERT(m_pImpl.get() == NULL);

    m_activeRequestsCount = 0;

    boost::asio::ip::tcp::endpoint serverEndpoint(boost::asio::ip::tcp::v4(), m_port);
    serverEndpoint.address(boost::asio::ip::address::from_string(m_host));

    m_pImpl.reset(new WebServerImpl(serverEndpoint, getHttpThreadCount(), getTimeout(), getConnectionLimit()));
    m_workSchedulerPtr.reset(new Scheduler(getWorkerThreadCount(), getWorkerThreadCount()));

    for(Services::iterator i = m_services.begin(); i != m_services.end(); ++i)
    {
        i->second->start();
    }

    for(Services::iterator i = m_services.begin(); i != m_services.end(); ++i)
    {
        ServiceHandlerPtr handlerPtr(new ServiceHandler(i->second,
                                                        m_workSchedulerPtr,
                                                        m_statPtr,
                                                        ServiceHandler::ErrorHandler(boost::bind(&WebServer::onHandlerError, this, _1, _2, _3)),
                                                        getConnectionLimit(),
                                                        m_activeRequestsCount));

        pion::http::server::request_handler_t handler = boost::bind(&ServiceHandler::operator(), handlerPtr, _1, _2);

        m_pImpl->m_pionWebServerCorePtr->add_resource(i->first, handler);
    }

    for (PluginServices::iterator i = m_pluginServices.begin(); i != m_pluginServices.end(); ++i)
    {
        m_pImpl->m_pionWebServerCorePtr->add_service(i->first, i->second.first.get());
        const PluginServiceOptions &opts = i->second.second;

        for (PluginServiceOptions::const_iterator j = opts.begin(); j != opts.end(); ++j)
        {
            m_pImpl->m_pionWebServerCorePtr->set_service_option(i->first, j->first, j->second);
        }
    }

    for(std::map<std::string, std::string>::const_iterator i = m_redirects.begin(); i != m_redirects.end(); ++i)
    {
        m_pImpl->m_pionWebServerCorePtr->add_redirect(i->first, i->second);
    }

    m_pImpl->m_pionWebServerCorePtr->set_authentication(m_authPtr);

    m_workSchedulerPtr->start();
    m_pImpl->m_pionWebServerCorePtr->start();
    setRunning(true);
}

//--------------------------------------------------------------------------------------------------
void WebServer::addRedirect(const std::string &from, const std::string &to)
{
    m_redirects[from] = to;
}

//--------------------------------------------------------------------------------------------------
void WebServer::stop()
{
    if (!isRunning())
    {
        throw WebServerError("WebServer is not running");
    }
    m_pImpl->m_pionWebServerCorePtr->stop(true);
    m_workSchedulerPtr->stop();

    m_pImpl.reset();

    Services::iterator i = m_services.begin();
    for(; i != m_services.end(); ++i)
    {
        i->second->stop();
    }

    m_activeRequestsCount = 0;
    setRunning(false);
}

//--------------------------------------------------------------------------------------------------
IStatPtr WebServer::getStatService()
{
    if (!m_enableStat)
    {
        throw WebServerError("Stat service not enabled.");
    }
    return m_statPtr;
}

//--------------------------------------------------------------------------------------------------
void WebServer::enableStatService(const std::string &serviceName,
                                  const std::string &resource,
                                  const std::string &version,
                                  const std::string &revision)
{
    m_enableStat = true;
    m_statServiceName = serviceName;
    m_statResource = resource;

    boost::shared_ptr<StatService> statServicePtr(new StatService(serviceName,
                                                                  resource,
                                                                  version,
                                                                  revision));
    addService(resource, statServicePtr);
    m_statPtr = statServicePtr;
}

//--------------------------------------------------------------------------------------------------
void WebServer::enableConfService(boost::function<std::pair<std::string,std::string>()> confCallback,
                       const std::string &resource)
{
    boost::shared_ptr<ConfService> confServicePtr(new ConfService(confCallback));
    this->addService(resource, confServicePtr);

}

//--------------------------------------------------------------------------------------------------
std::size_t WebServer::getWorkerThreadCount() const
{
    return m_workerThreadCount;
}

//--------------------------------------------------------------------------------------------------
void WebServer::setWorkerThreadCount(std::size_t workerThreadCount)
{
    m_workerThreadCount = workerThreadCount;
}

//--------------------------------------------------------------------------------------------------
void WebServer::onHandlerError(pion::http::request_ptr requestPtr,
                               pion::tcp::connection_ptr tcpConnPtr,
                               const std::exception &e)
{
    Logger::Logger &logger = Logger::Logger::getInstance();
    logger.error("Processing request \"" + requestPtr->get_query_string() + "\". Error: " + e.what());

    pion::http::server::handle_server_error(requestPtr, tcpConnPtr, e.what());
}

//--------------------------------------------------------------------------------------------------
void WebServer::setAuth(pion::http::auth_ptr authPtr)
{
    m_authPtr = authPtr;
}

} /* namespace WebServer */
} /* namespace Tools */

