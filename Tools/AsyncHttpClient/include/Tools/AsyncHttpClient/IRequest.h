#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUEST_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUEST_H_

// Boost
#include <boost/asio/ip/tcp.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

// Pion
#include <pion/http/request.hpp>
#include <pion/http/response.hpp>

namespace Tools
{
namespace AsyncHttpClient
{

class IRequest;
typedef boost::shared_ptr<IRequest> IRequestPtr;

typedef boost::function<void(IRequestPtr, pion::http::response_ptr, const boost::system::error_code &)> FinishedHandler;

class IRequest : boost::noncopyable
{
public:
    virtual ~IRequest()
    {
    }

    virtual void operator()() = 0;
    virtual void cancel() = 0;
    virtual pion::http::request_ptr getRequest() = 0;
    virtual const boost::asio::ip::tcp::endpoint &getEndpoint() const = 0;
    virtual bool keepAlive() const = 0;
};

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUEST_H_ */
