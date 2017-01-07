// BOOST
#include <boost/bind.hpp>

// PION
#include <pion/http/response_writer.hpp>

#include "Tools/WebServer/ConnectionContext.h"

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
ConnectionContext::ConnectionContext(pion::http::request_ptr requestPtr,
                                     pion::tcp::connection_ptr tcpConnPtr,
                                     Tools::WebServer::IStatPtr statPtr,
                                     boost::atomic_int32_t &activeRequestsCount):
                                             m_requestPtr(requestPtr),
                                             m_tcpConnPtr(tcpConnPtr),
                                             m_statPtr(statPtr),
                                             m_responseSet(false),
                                             m_activeRequestsCount(activeRequestsCount)
{
    m_activeRequestsCount.fetch_add(1, boost::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
ConnectionContext::~ConnectionContext()
{
    // TODO: check isResponseSet()
    m_activeRequestsCount.fetch_add(-1, boost::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
pion::http::request_ptr ConnectionContext::getRequest()
{
    return m_requestPtr;
}

//--------------------------------------------------------------------------------------------------
pion::tcp::connection_ptr ConnectionContext::getTcpConn()
{
    return m_tcpConnPtr;
}

//--------------------------------------------------------------------------------------------------
Tools::WebServer::IStatPtr ConnectionContext::getStat()
{
    return m_statPtr;
}

//--------------------------------------------------------------------------------------------------
void ConnectionContext::sendResponse(pion::http::response_ptr responsePtr)
{
    if (isResponseSet())
    {
        throw std::runtime_error("ConnectionContext::sendResponse() invoked twice");
    }

    pion::http::response_writer_ptr writer(pion::http::response_writer::create(
            m_tcpConnPtr,
            responsePtr,
            boost::bind(&pion::tcp::connection::finish, m_tcpConnPtr)));

    writer->send();

    m_tcpConnPtr.reset();
    m_responseSet = true;
}

//--------------------------------------------------------------------------------------------------
bool ConnectionContext::isResponseSet() const
{
    return m_responseSet;
}

} /* namespace WebServer */
} /* namespace Tools */
