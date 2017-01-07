// This
#include "Tools/AsyncHttpClient/ConnectionPool.h"

namespace Tools
{
namespace AsyncHttpClient
{

//--------------------------------------------------------------------------------------------------
ConnectionPool::ConnectionPool():
            m_maxSize(0u)
{
}

//--------------------------------------------------------------------------------------------------
ConnectionPool::ConnectionPool(std::size_t maxSize,
                               const boost::shared_ptr<pion::scheduler> &schedulerPtr):
        m_pionSchedulerPtr(schedulerPtr), m_maxSize(maxSize)
{
}

//--------------------------------------------------------------------------------------------------
ConnectionPool::~ConnectionPool()
{
}

//--------------------------------------------------------------------------------------------------
pion::scheduler &ConnectionPool::getScheduler()
{
    return *m_pionSchedulerPtr;
}

//--------------------------------------------------------------------------------------------------
pion::tcp::connection_ptr ConnectionPool::getConnection(const boost::asio::ip::tcp::endpoint &endpoint,
                                                        bool keepAlive)
{
    if (keepAlive)
    {
        boost::lock_guard<boost::mutex> lock(m_connPoolMutex);
        Connections::iterator it = m_connections.lower_bound(endpoint);
        if (it != m_connections.end() && it->first == endpoint)
        {
            pion::tcp::connection_ptr connPtr = it->second;
            m_connections.erase(it);
            return connPtr;
        }
    }

    pion::tcp::connection_ptr newConnPtr(new pion::tcp::connection(getScheduler().get_io_service()));
    newConnPtr->set_lifecycle(keepAlive ?
            pion::tcp::connection::LIFECYCLE_KEEPALIVE : pion::tcp::connection::LIFECYCLE_CLOSE);

    return newConnPtr;
}

//--------------------------------------------------------------------------------------------------
void ConnectionPool::putConnection(pion::tcp::connection_ptr connPtr)
{
    const bool keepAlive = (connPtr->get_lifecycle() == pion::tcp::connection::LIFECYCLE_KEEPALIVE);
    if (!keepAlive || !connPtr->is_open())
    {
        if (connPtr->is_open())
        {
            connPtr->close();
        }
        return;
    }

    boost::lock_guard<boost::mutex> lock(m_connPoolMutex);
    if (m_connections.size() >= getMaxSize())
    {
        connPtr->close();
        return;
    }

    const boost::asio::ip::tcp::endpoint endpoint = connPtr->get_remote_endpoint();

    const std::pair<Connections::iterator, Connections::iterator> range =
            m_connections.equal_range(endpoint);

    if (range.first->first == endpoint)
    {
        for(Connections::iterator it = range.first; it != range.second; ++it)
        {
            if (it->second == connPtr)
            {
                connPtr->close();
                m_connections.erase(it);
                return;
            }
        }
    }

    m_connections.insert(Connections::value_type(endpoint, connPtr));
}

//--------------------------------------------------------------------------------------------------
std::size_t ConnectionPool::getMaxSize() const
{
    return m_maxSize;
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */
