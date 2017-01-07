#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUEST_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUEST_H_

// Boost
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "Tools/Logger/Logger.h"

// This
#include "Tools/AsyncHttpClient/IConnectionPool.h"
#include "Tools/AsyncHttpClient/IRequest.h"

namespace Tools
{
namespace AsyncHttpClient
{

class Request : public IRequest, public boost::enable_shared_from_this<Request>
{
public:
    Request(pion::http::request_ptr request,
            const boost::asio::ip::tcp::endpoint &endpoint,
            const FinishedHandler &finishedHandler,
            IConnectionPoolPtr connPool,
            bool keepAlive);

    virtual ~Request();

    // IRequest
    virtual void operator()();
    virtual void cancel();
    virtual pion::http::request_ptr getRequest();
    virtual const boost::asio::ip::tcp::endpoint &getEndpoint() const;
    virtual bool keepAlive() const;

private:
    const FinishedHandler &getHandler() const;

    bool isCanceled() const;

    void setCanceled(bool canceled);

    void startConnection();

    void onConnect(const boost::system::error_code &error);

    void onRequestSent(const boost::system::error_code &error);

    void onResponseRead(pion::http::response_ptr responsePtr,
                        pion::tcp::connection_ptr connPtr,
                        const boost::system::error_code &error);

    void onResponse(pion::http::response_ptr responsePtr, const boost::system::error_code &ec);

    void onCancel();

    void onError(const boost::system::error_code &error,
                 const std::string &prefix,
                 const std::string &logMsg);

    std::string getAddressString() const;

    std::string getURL();

    static Tools::Logger::Logger &logger();

    pion::http::request_ptr m_requestPtr;
    boost::asio::ip::tcp::endpoint m_endpoint;
    FinishedHandler m_finishedHandler;
    IConnectionPoolPtr m_connPoolPtr;
    bool m_keepAlive;

    pion::tcp::connection_ptr m_connPtr;
    boost::mutex m_connMutex;
    volatile bool m_canceled;
};

typedef boost::shared_ptr<Request> RequestPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_REQUEST_H_ */
