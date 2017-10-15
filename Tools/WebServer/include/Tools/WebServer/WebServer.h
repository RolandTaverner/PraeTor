#ifndef WEBSERVER_H_
#define WEBSERVER_H_

// C++
#include <map>
#include <string>
#include <utility>

// Boost
#include <boost/atomic.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

// Pion
#include <pion/http/auth.hpp>
#include <pion/http/plugin_service.hpp>

#include "Tools/WebServer/Errors.h"
#include "Tools/WebServer/IStat.h"
#include "Tools/WebServer/IWebService.h"
#include "Tools/WebServer/Scheduler.h"

namespace Tools
{
namespace WebServer
{

typedef boost::shared_ptr<pion::http::plugin_service> PluginServicePtr;
typedef std::map<std::string, std::string> PluginServiceOptions;

class WebServer : boost::noncopyable
{
public:
    WebServer();
    WebServer(const std::string &host,
              const unsigned int port,
              const std::size_t httpThreadCount,
              const std::size_t timeout,
              const std::size_t connectionLimit,
              const std::size_t workerThreadCount);

    virtual ~WebServer();

    void start();
    void stop();

    const std::string &getHost() const;
    void setHost(const std::string &host);
    unsigned int getPort() const;
    void setPort(unsigned int port);
    std::size_t getHttpThreadCount() const;
    void setHttpThreadCount(std::size_t threadCount);
    std::size_t getTimeout() const;
    void setTimeout(std::size_t timeout);
    std::size_t getConnectionLimit() const;
    void setConnectionLimit(std::size_t connectionLimit);
    std::size_t getWorkerThreadCount() const;
    void setWorkerThreadCount(std::size_t workerThreadCount);
    bool isRunning() const;
    void enableStatService(const std::string &serviceName,
                           const std::string &resource,
                           const std::string &version,
                           const std::string &revision);
    void enableConfService(boost::function<std::pair<std::string,std::string>()> confCallback,
                           const std::string &resource = "/conf");
    IStatPtr getStatService();
    void addService(const std::string &resource, IWebServicePtr servicePtr);
    void addPluginService(const std::string &resource,
                          PluginServicePtr servicePtr,
                          const PluginServiceOptions &options);
    void addRedirect(const std::string &from, const std::string &to);
    void setAuth(pion::http::auth_ptr authPtr);

protected:
    void onHandlerError(pion::http::request_ptr requestPtr,
                        pion::tcp::connection_ptr tcpConnPtr,
                        const std::exception &e);

private:
    void setRunning(bool isRunning);

    struct WebServerImpl;
    boost::scoped_ptr<WebServerImpl> m_pImpl;
    boost::atomic_int32_t m_activeRequestsCount;

    SchedulerPtr m_workSchedulerPtr;

    std::string m_host;
    unsigned int m_port;
    std::size_t m_httpThreadCount;
    std::size_t m_timeout;
    std::size_t m_connectionLimit;
    std::size_t m_workerThreadCount;
    bool m_isRunning;

    typedef std::map<std::string, IWebServicePtr> Services;
    Services m_services;

    typedef std::pair<PluginServicePtr, PluginServiceOptions> PluginServiceDesc;
    typedef std::map<std::string, PluginServiceDesc> PluginServices;
    PluginServices m_pluginServices;

    std::map<std::string, std::string> m_redirects;

    // stat options
    bool m_enableStat;
    std::string m_statServiceName;
    std::string m_statResource;
    IStatPtr m_statPtr;

    // trace options
    std::string m_serviceName;

    // auth object
    pion::http::auth_ptr m_authPtr;
};

} /* namespace WebServer */
} /* namespace Tools */

#endif /* WEBSERVER_H_ */
