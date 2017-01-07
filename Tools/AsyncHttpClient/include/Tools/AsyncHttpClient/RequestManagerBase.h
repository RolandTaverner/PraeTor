#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUESTMANAGERBASE_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUESTMANAGERBASE_H_

// C++
#include <map>
#include <stdexcept>
#include <utility>

// Boost
#include <boost/atomic.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/mutex.hpp>

// Pion
#include <pion/http/request.hpp>

// This
#include "Tools/AsyncHttpClient/HandlerTypes.h"
#include "Tools/AsyncHttpClient/IConnectionPool.h"
#include "Tools/AsyncHttpClient/IRequest.h"
#include "Tools/AsyncHttpClient/IRequestManager.h"

namespace Tools
{
namespace AsyncHttpClient
{

class RequestManagerBase :
        public IRequestManager,
        public boost::enable_shared_from_this<RequestManagerBase>
{
public:
    typedef std::pair<pion::http::response_ptr, boost::system::error_code> ResponseValue;
    typedef std::map<IRequestPtr, ResponseValue> RequestResponse;

public:
    RequestManagerBase(const IConnectionPoolPtr &connPoolPtr,
                       const boost::shared_ptr<pion::scheduler> &schedulerPtr);
    virtual ~RequestManagerBase();

    // IRequestManager
    virtual void start(const boost::posix_time::time_duration &timeout);

    virtual void cancel();

protected:
    virtual void addRequest(pion::http::request_ptr request,
                            const boost::asio::ip::tcp::endpoint &endpoint,
                            bool keepAlive);

    virtual void onFinished(const RequestResponse &results) = 0;

    std::size_t getRequestsCount();

private:
    void onRequestFinished(IRequestPtr requestPtr,
                           pion::http::response_ptr responsePtr,
                           const boost::system::error_code &ec);

    void onTimer(const boost::system::error_code &error);

    void cancelPendingRequests();

private:
    IConnectionPoolPtr m_connPoolPtr;
    boost::shared_ptr<pion::scheduler> m_schedulerPtr;

    boost::mutex m_mutex;
    boost::shared_ptr<boost::asio::deadline_timer> m_timerPtr;
    boost::atomic_bool m_timerCompleted;

    RequestResponse m_requestResponse;

    boost::atomic_uint32_t m_pendingRequestsCount;
};

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUESTMANAGERBASE_H_ */
