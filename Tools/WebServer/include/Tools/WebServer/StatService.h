#ifndef STATSERVICE_H_
#define STATSERVICE_H_

// C++
#include <map>
#include <stdexcept>

// BOOST
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread/mutex.hpp>

#include "Tools/WebServer/IStat.h"
#include "Tools/WebServer/IWebService.h"

namespace Tools
{
namespace WebServer
{

class StatService : public Tools::WebServer::IStat, public Tools::WebServer::IWebService
{
public:
    explicit StatService(const std::string &serviceName,
                         const std::string &resource,
                         const std::string &version,
                         const std::string &revision);
    virtual ~StatService();

    // IWebService overloads
    virtual void operator()(ConnectionContextPtr contextPtr);
    virtual void start(void);
    virtual void stop(void);

    // Tools::WebServer::IStat overloads
    virtual void registerParameter(const std::string &name);
    virtual void set(const std::string &name, const long value);
    virtual void increment(const std::string &name, const long value);
    virtual void registerParametersProvider(const std::string &name,
                                            const ParametersProvider &parametersProvider);
    virtual void unregisterParametersProvider(const std::string &name);

    bool isRunning() const;

private:
    typedef std::map<std::string, unsigned> ParameterIDs;
    typedef std::map<std::string, ParametersProvider> ParametersProviders;

    volatile bool m_isRunning;
    ParameterIDs m_parameterIDs;
    std::vector<long> m_parameterValues;
    std::string m_serviceName;
    std::string m_resource;

    mutable boost::mutex m_mutexProvider;
    ParametersProviders m_parametersProviders;

    // built-in values
    boost::posix_time::ptime m_startTime;
    std::string m_serviceVersion;
    std::string m_serviceRevision;
};

} /* namespace WebServer */
} /* namespace Tools */
#endif /* STATSERVICE_H_ */
