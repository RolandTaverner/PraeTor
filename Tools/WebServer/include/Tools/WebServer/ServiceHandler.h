#ifndef SERVICEHANDLER_H_
#define SERVICEHANDLER_H_

// Boost
#include <boost/atomic.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

// Pion
#include <pion/http/request.hpp>
#include <pion/tcp/connection.hpp>

#include "Tools/Logger/Logger.h"
#include "Tools/WebServer/ConnectionContext.h"
#include "Tools/WebServer/IWebService.h"
#include "Tools/WebServer/IScheduler.h"

namespace Tools
{
namespace WebServer
{

class ServiceHandler : boost::noncopyable
{
public:
    typedef boost::function<void(pion::http::request_ptr, pion::tcp::connection_ptr, const std::exception &)> ErrorHandler;

    ServiceHandler(IWebServicePtr servicePtr,
                   ISchedulerPtr schedulerPtr,
                   IStatPtr statPtr,
                   ErrorHandler errorHandler,
                   boost::int32_t maxActiveRequests,
                   boost::atomic_int32_t &activeRequestsCount);

    virtual ~ServiceHandler();

    void operator()(pion::http::request_ptr &requestPtr, pion::tcp::connection_ptr &tcpConnPtr);

    void invokeHandler(ConnectionContextPtr contextPtr);

    inline boost::int32_t getMaxActiveRequests() const;

private:
    static Tools::Logger::Logger &logger();
    static bool checkID(const std::string &strID);

    boost::int64_t getNewTraceID();

    boost::atomic_int32_t &m_activeRequestsCount;
    boost::int32_t m_maxActiveRequests;
    IWebServicePtr m_servicePtr;
    ISchedulerPtr m_schedulerPtr;
    IStatPtr m_statPtr;
    ErrorHandler m_errorHandler;
};

typedef boost::shared_ptr<ServiceHandler> ServiceHandlerPtr;

} /* namespace WebServer */
} /* namespace Tools */

#endif /* SERVICEHANDLER_H_ */
