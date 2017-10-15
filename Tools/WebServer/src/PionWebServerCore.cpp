// PION
#include <pion/http/request_reader.hpp>
#include <pion/logger.hpp>

#include "Tools/WebServer/PionWebServerCore.h"

namespace Tools
{
namespace WebServer
{
namespace Detail
{

//--------------------------------------------------------------------------------------------------
PionWebServerCore::PionWebServerCore(pion::scheduler &scheduler,
                                     const boost::asio::ip::tcp::endpoint &endpoint,
                                     std::size_t timeout,
                                     std::size_t connectionLimit):
        pion::http::plugin_server(scheduler, endpoint),
        m_timeout(timeout),
        m_connectionLimit(connectionLimit)
{
#ifdef DEBUG
    PION_LOG_SETLEVEL_DEBUG(get_logger());
#endif
}

//--------------------------------------------------------------------------------------------------
PionWebServerCore::~PionWebServerCore()
{
}

//--------------------------------------------------------------------------------------------------
std::size_t PionWebServerCore::getConnectionLimit() const
{
    return m_connectionLimit;
}

//--------------------------------------------------------------------------------------------------
std::size_t PionWebServerCore::getTimeout() const
{
    return m_timeout;
}

//--------------------------------------------------------------------------------------------------
void PionWebServerCore::handle_connection(pion::tcp::connection_ptr &tcpConnPtr)
{
//TODO: avoid deadlock with keep-alive connections
//    // Если лимит подключений достигнут, то текущее подключение разрывается
//    if (getConnections() > getConnectionLimit())
//    {
//        tcpConn->setLifecycle(pion::net::TCPConnection::LIFECYCLE_CLOSE);
//        tcpConn->close();
//        return;
//    }

    pion::http::request_reader::finished_handler_t handler(boost::bind(&PionWebServerCore::handle_request, this, _1, _2, _3));
    pion::http::request_reader_ptr readerPtr = pion::http::request_reader::create(tcpConnPtr, handler);

    readerPtr->set_timeout(getTimeout());
    readerPtr->receive();
}

void PionWebServerCore::handle_request(pion::http::request_ptr http_request_ptr,
                                       pion::tcp::connection_ptr tcp_conn,
                                       const boost::system::error_code& ec)
{
    pion::http::server::handle_request(http_request_ptr, tcp_conn, ec);
}

} /* namespace Detail */
} /* namespace WebServer */
} /* namespace Tools */
