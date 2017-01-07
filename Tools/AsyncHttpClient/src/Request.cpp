// C++
#include <sstream>

// Boost
//#include <boost/asio/socket_base.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

// Pion
#include <pion/http/request_writer.hpp>
#include <pion/http/response_reader.hpp>
#include <pion/http/types.hpp>

// This
#include "Tools/AsyncHttpClient/Error.h"
#include "Tools/AsyncHttpClient/Request.h"

namespace Tools
{
namespace AsyncHttpClient
{
namespace Detail
{

enum ErrCat { SystemGeneric = 1, AsioMisc };

//--------------------------------------------------------------------------------------------------
bool isErrorCategory(const boost::system::error_code &ec, const ErrCat errCat)
{
    switch (errCat)
    {
        case SystemGeneric:
        {
            return ec.category().name() == boost::system::get_generic_category().name();
        }; break;
        case AsioMisc:
        {
            return ec.category().name() == boost::asio::error::get_misc_category().name();
        }; break;
        default:
        {
            throw std::runtime_error("Unkown category code");
        }
    }
    return false;
}

} /* namespace Detail */

//--------------------------------------------------------------------------------------------------
Request::Request(pion::http::request_ptr request,
                 const boost::asio::ip::tcp::endpoint &endpoint,
                 const FinishedHandler &finishedHandler,
                 IConnectionPoolPtr connPool,
                 bool keepAlive) :
                m_requestPtr(request),
                m_endpoint(endpoint),
                m_finishedHandler(finishedHandler),
                m_connPoolPtr(connPool),
                m_keepAlive(keepAlive),
                m_canceled(false)
{
}

//--------------------------------------------------------------------------------------------------
Request::~Request()
{
}

//--------------------------------------------------------------------------------------------------
void Request::operator()()
{
    startConnection();
}

//--------------------------------------------------------------------------------------------------
void Request::cancel()
{
    boost::lock_guard<boost::mutex> lock(m_connMutex);

    if (m_connPtr)
    {
        setCanceled(true);
        m_connPtr->cancel();
    }
}

//--------------------------------------------------------------------------------------------------
pion::http::request_ptr Request::getRequest()
{
    return m_requestPtr;
}

//--------------------------------------------------------------------------------------------------
const boost::asio::ip::tcp::endpoint &Request::getEndpoint() const
{
    return m_endpoint;
}

//--------------------------------------------------------------------------------------------------
bool Request::keepAlive() const
{
    return m_keepAlive;
}

//--------------------------------------------------------------------------------------------------
const FinishedHandler &Request::getHandler() const
{
    return m_finishedHandler;
}

//--------------------------------------------------------------------------------------------------
void Request::startConnection()
{
    try
    {
        boost::lock_guard<boost::mutex> lock(m_connMutex);
        if (isCanceled())
        {
            onCancel();
            return;
        }
        m_connPtr = m_connPoolPtr->getConnection(getEndpoint(), keepAlive());
        if (!m_connPtr->is_open())
        {
            // New connection
            m_connPtr->async_connect(getEndpoint(),
                                     boost::bind(&Request::onConnect, shared_from_this(), _1));
            return;
        }
    }
    catch (const std::exception &e)
    {
        onResponse(pion::http::response_ptr(), Error::makeErrorCode(Error::internalError));
        logger().error("Starting request to " + getAddressString() + " failed: " + e.what());
        return;
    }

    // Connection from pool
    onConnect(boost::system::error_code());
}

//--------------------------------------------------------------------------------------------------
void Request::onConnect(const boost::system::error_code &error)
{
    BOOST_ASSERT(m_connPtr.get() != NULL);

    if (error)
    {
        onError(isCanceled() ? Error::makeErrorCode(Error::requestCanceled) : error,
                "tcp.connect",
                "Connect to " + getAddressString() + " failed: " + error.message());
        return;
    }

//    m_connPtr->get_socket().set_option(boost::asio::socket_base::keep_alive(keepAlive));

    // headers
    getRequest()->change_header(pion::http::types::HEADER_HOST, getAddressString());

    getRequest()->change_header(pion::http::types::HEADER_CONNECTION,
                                keepAlive() ? "Keep-Alive" : "close");

    {
        boost::lock_guard<boost::mutex> lock(m_connMutex);

        if (isCanceled())
        {
            onCancel();
            return;
        }

        // send request
        pion::http::request_writer_ptr httpRequestWriterPtr =
                pion::http::request_writer::create(m_connPtr,
                                                   m_requestPtr,
                                                   boost::bind<>(&Request::onRequestSent,
                                                                 shared_from_this(),
                                                                 _1));
        httpRequestWriterPtr->send();
    }
}

//--------------------------------------------------------------------------------------------------
void Request::onCancel()
{
    onResponse(pion::http::response_ptr(), Error::makeErrorCode(Error::requestCanceled));

    if (m_connPtr.get() != NULL)
    {
        m_connPtr->close();
        m_connPtr.reset();
    }
}

//--------------------------------------------------------------------------------------------------
void Request::onRequestSent(const boost::system::error_code &error)
{
    BOOST_ASSERT(m_connPtr.get() != NULL);

    if (error)
    {
        if (!isCanceled() && keepAlive()
                && ((Detail::isErrorCategory(error, Detail::SystemGeneric)
                                && error.value() == boost::system::errc::io_error)))
        {
            // Remote service closed connection unexpectedly.
            // Try again. Maybe remote service doesn't honor keep-alive header.
            {
                boost::lock_guard<boost::mutex> lock(m_connMutex);
                if (m_connPtr->is_open())
                {
                    m_connPtr->close();
                }
                m_connPtr.reset();
            }

            logger().warning("Sending request to " + getAddressString() + " failed: "
                    + error.message() + ". Try again.");

            startConnection();
        }
        else
        {
            onError(isCanceled() ? Error::makeErrorCode(Error::requestCanceled) : error,
                    "tcp.send",
                    "Send request to " + getAddressString() + " failed: " + error.message());
        }
        return;
    }

    {
        boost::lock_guard<boost::mutex> lock(m_connMutex);

        if (isCanceled())
        {
            onCancel();
            return;
        }

        pion::http::response_reader_ptr readerPtr =
                pion::http::response_reader::create(m_connPtr,
                                                    *getRequest(),
                                                    boost::bind<>(&Request::onResponseRead,
                                                                  shared_from_this(),
                                                                  _1,
                                                                  _2,
                                                                  _3));
        readerPtr->receive();
    }
}

//--------------------------------------------------------------------------------------------------
void Request::onResponseRead(pion::http::response_ptr responsePtr,
                             pion::tcp::connection_ptr connPtr,
                             const boost::system::error_code &error)
{
    BOOST_ASSERT(m_connPtr.get() != NULL);

    connPtr.reset();

    if (error)
    {
        if (!isCanceled() && keepAlive()
                && (Detail::isErrorCategory(error, Detail::AsioMisc)
                        && error.value() == boost::asio::error::eof))
        {
            // Remote service closed connection unexpectedly.
            // Try again. Maybe remote service doesn't honor keep-alive header.
            {
                boost::lock_guard<boost::mutex> lock(m_connMutex);
                if (m_connPtr->is_open())
                {
                    m_connPtr->close();
                }
                m_connPtr.reset();
            }

            logger().warning("Reading response from " + getAddressString() + " failed: "
                    + error.message() + ". Try again.");

            startConnection();
        }
        else
        {
            onError(isCanceled() ? Error::makeErrorCode(Error::requestCanceled) : error,
                    "tcp.recv",
                    "Read response from " + getAddressString() + " failed: " + error.message());
        }
        return;
    }

    {
        boost::lock_guard<boost::mutex> lock(m_connMutex);
        if (keepAlive() && responsePtr->check_keep_alive())
        {
            m_connPoolPtr->putConnection(m_connPtr);
        }
        else
        {
            m_connPtr->close();
        }
        m_connPtr.reset();
    }

    onResponse(responsePtr, boost::system::error_code());
}

//--------------------------------------------------------------------------------------------------
void Request::onResponse(pion::http::response_ptr responsePtr, const boost::system::error_code &ec)
{
    try
    {
        getHandler()(shared_from_this(), responsePtr, ec);
    }
    catch(const std::exception &e)
    {
        logger().error("Response handling from " + getAddressString() + " failed: " + e.what());
    }
}

//--------------------------------------------------------------------------------------------------
bool Request::isCanceled() const
{
    return m_canceled;
}

//--------------------------------------------------------------------------------------------------
void Request::setCanceled(bool canceled)
{
    m_canceled = canceled;
}

//--------------------------------------------------------------------------------------------------
std::string Request::getAddressString() const
{
    return getEndpoint().address().to_string() + ":"
            + boost::lexical_cast<std::string>(getEndpoint().port());
}

//--------------------------------------------------------------------------------------------------
std::string Request::getURL()
{
    std::string url = getAddressString() + getRequest()->get_original_resource();
    if (!getRequest()->get_query_string().empty())
    {
        url += "?" + getRequest()->get_query_string();
    }

    return url;
}

//--------------------------------------------------------------------------------------------------
void Request::onError(const boost::system::error_code &error,
                      const std::string &prefix,
                      const std::string &logMsg)
{
    onResponse(pion::http::response_ptr(), error);
    {
        boost::lock_guard<boost::mutex> lock(m_connMutex);
        m_connPtr->close();
        m_connPtr.reset();
    }

    logger().warning(logMsg);
}

//--------------------------------------------------------------------------------------------------
Tools::Logger::Logger &Request::logger()
{
    return Tools::Logger::Logger::getInstance();
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */
