#ifndef CONNECTIONCONTEXT_H_
#define CONNECTIONCONTEXT_H_

// BOOST
#include <boost/atomic.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

// PION
#include <pion/http/request.hpp>
#include <pion/http/response.hpp>
#include <pion/tcp/connection.hpp>

#include "Tools/WebServer/IStat.h"

namespace Tools
{
namespace WebServer
{

class ConnectionContext :
        public boost::enable_shared_from_this<ConnectionContext>,
        private boost::noncopyable
{
public:
    ConnectionContext(pion::http::request_ptr requestPtr,
                      pion::tcp::connection_ptr tcpConnPtr,
                      Tools::WebServer::IStatPtr statPtr,
                      boost::atomic_int32_t &activeRequestsCount);
    virtual ~ConnectionContext();

    pion::http::request_ptr getRequest();
    pion::tcp::connection_ptr getTcpConn();
    Tools::WebServer::IStatPtr getStat();
    void sendResponse(pion::http::response_ptr responsePtr);
    bool isResponseSet() const;

private:
    pion::http::request_ptr m_requestPtr;
    pion::tcp::connection_ptr m_tcpConnPtr;
    Tools::WebServer::IStatPtr m_statPtr;
    bool m_responseSet;
    boost::atomic_int32_t &m_activeRequestsCount;
};

typedef boost::shared_ptr<ConnectionContext> ConnectionContextPtr;

} /* namespace WebServer */
} /* namespace Tools */
#endif /* CONNECTIONCONTEXT_H_ */
