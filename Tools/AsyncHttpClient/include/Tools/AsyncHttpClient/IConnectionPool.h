#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ICONNECTIONPOOL_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ICONNECTIONPOOL_H_

// Boost
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

// Pion
#include <pion/scheduler.hpp>
#include <pion/tcp/connection.hpp>

namespace Tools
{
namespace AsyncHttpClient
{

class IConnectionPool : boost::noncopyable
{
public:
    virtual ~IConnectionPool(){}

    /** @return планировщик pion*/
    virtual pion::scheduler &getScheduler() = 0;

    virtual pion::tcp::connection_ptr getConnection(const boost::asio::ip::tcp::endpoint &endpoint,
                                                    bool keepAlive) = 0;

    virtual void putConnection(pion::tcp::connection_ptr connPtr) = 0;
};

typedef boost::shared_ptr<IConnectionPool> IConnectionPoolPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ICONNECTIONPOOL_H_ */
