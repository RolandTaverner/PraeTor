#ifndef PIONWEBSERVERCORE_H_
#define PIONWEBSERVERCORE_H_

#include <boost/shared_ptr.hpp>

#include <pion/http/server.hpp>

namespace Tools
{
namespace WebServer
{
namespace Detail
{

class PionWebServerCore : public pion::http::server
{
public:
    PionWebServerCore(pion::scheduler &scheduler,
                      const boost::asio::ip::tcp::endpoint &endpoint,
                      std::size_t timeout,
                      std::size_t connectionLimit);
    virtual ~PionWebServerCore();

    std::size_t getConnectionLimit() const;
    std::size_t getTimeout() const;

protected:
    virtual void handle_connection(pion::tcp::connection_ptr &tcpConnPtr);

    void handle_request(pion::http::request_ptr http_request_ptr,
                        pion::tcp::connection_ptr tcp_conn, 
                        const boost::system::error_code& ec);
private:
    std::size_t m_timeout;
    std::size_t m_connectionLimit;
};

typedef boost::shared_ptr<PionWebServerCore> PionWebServerCorePtr;

} /* namespace Detail */
} /* namespace WebServer */
} /* namespace Tools */
#endif /* PIONWEBSERVERCORE_H_ */
