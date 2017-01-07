#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_CONNECTIONPOOL_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_CONNECTIONPOOL_H_

// C++
#include <cstddef>
#include <map>
#include <string>

// Boost
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// This
#include "Tools/AsyncHttpClient/IConnectionPool.h"

namespace Tools
{
namespace AsyncHttpClient
{

class ConnectionPool : public IConnectionPool
{
private:
    ConnectionPool();
public:
    ConnectionPool(std::size_t maxSize, const boost::shared_ptr<pion::scheduler> &schedulerPtr);
    virtual ~ConnectionPool();

    // IConnectionPool
    virtual pion::scheduler &getScheduler();
    virtual pion::tcp::connection_ptr getConnection(const boost::asio::ip::tcp::endpoint &endpoint,
                                                    bool keepAlive);
    virtual void putConnection(pion::tcp::connection_ptr connPtr);

    std::size_t getMaxSize() const;

private:
    boost::shared_ptr<pion::scheduler> m_pionSchedulerPtr;

    typedef std::multimap<boost::asio::ip::tcp::endpoint, pion::tcp::connection_ptr> Connections;

    Connections m_connections;
    std::size_t m_maxSize;
    boost::mutex m_connPoolMutex;
};

typedef boost::shared_ptr<ConnectionPool> ConnectionPoolPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_CONNECTIONPOOL_H_ */
