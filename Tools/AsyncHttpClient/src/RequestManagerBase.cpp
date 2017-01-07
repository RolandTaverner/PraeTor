// Boost
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

// This
#include "Tools/AsyncHttpClient/Error.h"
#include "Tools/AsyncHttpClient/Request.h"
#include "Tools/AsyncHttpClient/RequestManagerBase.h"

namespace Tools
{
namespace AsyncHttpClient
{

//--------------------------------------------------------------------------------------------------
RequestManagerBase::RequestManagerBase(const IConnectionPoolPtr &connPoolPtr,
                                       const boost::shared_ptr<pion::scheduler> &schedulerPtr):
                m_connPoolPtr(connPoolPtr),
                m_schedulerPtr(schedulerPtr),
                m_timerCompleted(false),
                m_pendingRequestsCount(0u)
{
    BOOST_ASSERT(m_connPoolPtr.get() != NULL);
    BOOST_ASSERT(m_schedulerPtr.get() != NULL);
}

//--------------------------------------------------------------------------------------------------
RequestManagerBase::~RequestManagerBase()
{
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::addRequest(pion::http::request_ptr request,
                                    const boost::asio::ip::tcp::endpoint &endpoint,
                                    bool keepAlive)
{
    boost::lock_guard<boost::mutex> lock(m_mutex);

    IRequestPtr requestPtr(new Request(request,
                                       endpoint,
                                       boost::bind<>(&RequestManagerBase::onRequestFinished, shared_from_this(), _1, _2, _3),
                                       m_connPoolPtr,
                                       keepAlive));

    m_requestResponse[requestPtr] = ResponseValue();
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::start(const boost::posix_time::time_duration &timeout)
{
    if (m_requestResponse.empty())
    {
        throw std::runtime_error("Can't start request (no requests added)");
    }

    boost::lock_guard<boost::mutex> lock(m_mutex);

    if (m_pendingRequestsCount || m_timerCompleted || m_timerPtr.get() != NULL)
    {
        throw std::runtime_error("Request already running");
    }

    if (timeout.total_nanoseconds() > 0)
    {
        m_timerPtr.reset(new boost::asio::deadline_timer(m_schedulerPtr->get_io_service(),
                                                         timeout));
        m_timerPtr->async_wait(boost::bind<>(&RequestManagerBase::onTimer, shared_from_this(), _1));
    }

    BOOST_FOREACH(RequestResponse::value_type &request, m_requestResponse)
    {
        m_pendingRequestsCount.fetch_add(1, boost::memory_order_relaxed);
        m_schedulerPtr->post(boost::bind(&IRequest::operator(), request.first));
    }
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::onRequestFinished(IRequestPtr requestPtr,
                                           pion::http::response_ptr responsePtr,
                                           const boost::system::error_code &ec)
{
    boost::lock_guard<boost::mutex> lock(m_mutex);

    BOOST_ASSERT(m_pendingRequestsCount.load() >= 1u);

    const bool timerCompleted = m_timerCompleted.load(boost::memory_order_acquire);
    const boost::uint32_t pendingRequests = m_pendingRequestsCount.fetch_sub(1u, boost::memory_order_acq_rel);

    if (!timerCompleted && m_timerPtr.get() == NULL) // no timer
    {
        m_requestResponse[requestPtr] = ResponseValue(responsePtr, ec);
        if (pendingRequests == 1u)
        {
            onFinished(m_requestResponse);
            m_requestResponse.clear();
        }
    }
    else // have timer
    {
        if (!timerCompleted)
        {
            m_requestResponse[requestPtr] = ResponseValue(responsePtr, ec);

            if (pendingRequests == 1u) // this is last request
            {
                m_timerPtr->cancel();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::onTimer(const boost::system::error_code &error)
{
    boost::lock_guard<boost::mutex> lock(m_mutex);

    m_timerCompleted.store(true, boost::memory_order_release);

    if (error.value() == boost::asio::error::operation_aborted
            && error.category().name() == boost::asio::error::get_system_category().name())
    {
        // Do nothing, all ok
        BOOST_ASSERT(m_pendingRequestsCount.load() == 0u);
    }
    else
    {
        if (m_pendingRequestsCount.load() != 0u)
        {
            cancelPendingRequests();

            BOOST_FOREACH(RequestResponse::value_type &request, m_requestResponse)
            {
                if (request.second.first.get() == NULL && !request.second.second)
                {
                    request.second.second = Error::makeErrorCode(Error::requestTimedOut);
                }
            }
        }
    }

    onFinished(m_requestResponse);

    // cleanup
    m_timerPtr.reset();
    m_requestResponse.clear();
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::cancelPendingRequests()
{
    BOOST_FOREACH(RequestResponse::value_type &request, m_requestResponse)
    {
        if (request.second.first.get() == NULL && !request.second.second)
        {
            request.first->cancel();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void RequestManagerBase::cancel()
{
    boost::lock_guard<boost::mutex> lock(m_mutex);
    cancelPendingRequests();
}

//--------------------------------------------------------------------------------------------------
std::size_t RequestManagerBase::getRequestsCount()
{
    boost::lock_guard<boost::mutex> lock(m_mutex);
    return m_requestResponse.size();
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */

