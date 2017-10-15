#include "Tools/WebServer/RedirectService.h"

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
RedirectService::RedirectService(const std::string &from, const std::string &to, int httpStatus):
    m_from(from), m_to(to), m_httpStatus(httpStatus)
{
    m_httpStatusMessage[300] = "Multiple Choices";
    m_httpStatusMessage[301] = "Moved Permanently";
    m_httpStatusMessage[302] = "Found";
    m_httpStatusMessage[303] = "See Other";
    m_httpStatusMessage[304] = "Not Modified";
    m_httpStatusMessage[305] = "Use Proxy";
    m_httpStatusMessage[307] = "Temporary Redirect";
    m_httpStatusMessage[308] = "Permanent Redirect";
}

//--------------------------------------------------------------------------------------------------
RedirectService::~RedirectService()
{
}

//--------------------------------------------------------------------------------------------------
void RedirectService::operator()(Tools::WebServer::ConnectionContextPtr contextPtr)
{
    pion::http::request_ptr requestPtr = contextPtr->getRequest();

    pion::http::response_ptr responsePtr(new pion::http::response(requestPtr->get_method()));
    responsePtr->set_status_code(m_httpStatus);
    responsePtr->set_status_message(getStatusMessage(m_httpStatus));
    responsePtr->add_header("Location", m_to);
    
    contextPtr->sendResponse(responsePtr);
}

//--------------------------------------------------------------------------------------------------
void RedirectService::start(void)
{
}

//--------------------------------------------------------------------------------------------------
void RedirectService::stop(void)
{
}

//--------------------------------------------------------------------------------------------------
const std::string &RedirectService::getStatusMessage(unsigned statusCode) const
{
    std::map<unsigned, std::string>::const_iterator i = m_httpStatusMessage.find(statusCode);
    if (i == m_httpStatusMessage.end())
    {
        throw std::range_error("Can't find status message for code " + boost::lexical_cast<std::string>(statusCode));
    }
    return i->second;
}

} /* namespace WebServer */
} /* namespace Tools */
