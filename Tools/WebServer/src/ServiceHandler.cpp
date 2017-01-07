// C++
#include <iomanip>
#include <sstream>
#include <stdexcept>

// BOOST
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include "Tools/WebServer/ServiceHandler.h"

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
ServiceHandler::ServiceHandler(IWebServicePtr servicePtr,
                               ISchedulerPtr schedulerPtr,
                               IStatPtr statPtr,
                               ErrorHandler errorHandler,
                               boost::int32_t maxActiveRequests,
                               boost::atomic_int32_t &activeRequestsCount):
        m_activeRequestsCount(activeRequestsCount),
        m_maxActiveRequests(maxActiveRequests),
        m_servicePtr(servicePtr),
        m_schedulerPtr(schedulerPtr),
        m_statPtr(statPtr),
        m_errorHandler(errorHandler)
{
}

//--------------------------------------------------------------------------------------------------
ServiceHandler::~ServiceHandler()
{
}

//--------------------------------------------------------------------------------------------------
void ServiceHandler::operator()(pion::http::request_ptr &requestPtr,
                                pion::tcp::connection_ptr &tcpConnPtr)
{
    const boost::int32_t activeRequestsCount = m_activeRequestsCount.load(boost::memory_order_acquire);

    if (activeRequestsCount > getMaxActiveRequests())
    {
        m_errorHandler(requestPtr, tcpConnPtr, std::runtime_error("server overloaded"));
        return;
    }

    ConnectionContextPtr contextPtr(new ConnectionContext(requestPtr,
                                                          tcpConnPtr,
                                                          m_statPtr,
                                                          m_activeRequestsCount));

    try
    {
        m_schedulerPtr->execute(boost::bind(&ServiceHandler::invokeHandler, this, contextPtr));
    }
    catch (const std::exception &e)
    {
        m_errorHandler(requestPtr, tcpConnPtr, e);
    }
}

//--------------------------------------------------------------------------------------------------
inline boost::int32_t ServiceHandler::getMaxActiveRequests() const
{
    return m_maxActiveRequests;
}

//--------------------------------------------------------------------------------------------------
void ServiceHandler::invokeHandler(ConnectionContextPtr contextPtr)
{
    try
    {
        (*m_servicePtr)(contextPtr);
    }
    catch (const std::exception &e)
    {
        m_errorHandler(contextPtr->getRequest(), contextPtr->getTcpConn(), e);
    }
}

//--------------------------------------------------------------------------------------------------
Tools::Logger::Logger &ServiceHandler::logger()
{
    return Tools::Logger::Logger::getInstance();
}

} /* namespace WebServer */
} /* namespace Tools */
