#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_HANDLERTYPES_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_HANDLERTYPES_H_

// C++
#include <list>

// Boost
#include <boost/asio/ip/tcp.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/system/error_code.hpp>

// Pion
#include <pion/http/request.hpp>
#include <pion/http/response.hpp>

namespace Tools
{
namespace AsyncHttpClient
{

typedef boost::tuple<boost::asio::ip::tcp::endpoint,
                     pion::http::request_ptr,
                     pion::http::response_ptr,
                     boost::system::error_code> RequestResult;

typedef boost::function<void (const RequestResult &)> SingleRequestHandler;

typedef std::list<RequestResult> RequestResults;

typedef boost::function<void (const RequestResults &)> MultiRequestHandler;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_HANDLERTYPES_H_ */
