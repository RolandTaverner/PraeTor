#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <pion/algorithm.hpp>

#include <json/value.h>
#include <json/writer.h>

#include "Tools/CompressUtils/Compress.h"
#include "Tools/Logger/Logger.h"
#include "Tools/StringUtils/StringEscapeUtils.h"
#include "Tools/StringUtils/UnicodeTextProcessing.h"

#include "WebServices/ControllerAPIWebService.h"

static const char *s_resourceScheme =
"{" \
"}";

//-------------------------------------------------------------------------------------------------
ControllerAPIWebService::ControllerAPIWebService(ControllerPtr controller) :
    m_controller(controller), m_parser(s_resourceScheme)
{
    m_httpStatusMessage[100] = "Continue";
    m_httpStatusMessage[101] = "Switching Protocols";
    m_httpStatusMessage[102] = "Processing";
    m_httpStatusMessage[200] = "OK";
    m_httpStatusMessage[201] = "Created";
    m_httpStatusMessage[202] = "Accepted";
    m_httpStatusMessage[203] = "Non-Authoritative Information";
    m_httpStatusMessage[204] = "No Content";
    m_httpStatusMessage[205] = "Reset Content";
    m_httpStatusMessage[206] = "Partial Content";
    m_httpStatusMessage[207] = "Multi-Status";
    m_httpStatusMessage[208] = "Already Reported";
    m_httpStatusMessage[226] = "IM Used";
    m_httpStatusMessage[300] = "Multiple Choices";
    m_httpStatusMessage[301] = "Moved Permanently";
    m_httpStatusMessage[302] = "Found";
    m_httpStatusMessage[303] = "See Other";
    m_httpStatusMessage[304] = "Not Modified";
    m_httpStatusMessage[305] = "Use Proxy";
    m_httpStatusMessage[306] = "Reserved";
    m_httpStatusMessage[307] = "Temporary Redirect";
    m_httpStatusMessage[308] = "Permanent Redirect";
    m_httpStatusMessage[400] = "Bad Request";
    m_httpStatusMessage[401] = "Unauthorized";
    m_httpStatusMessage[402] = "Payment Required";
    m_httpStatusMessage[403] = "Forbidden";
    m_httpStatusMessage[404] = "Not Found";
    m_httpStatusMessage[405] = "Method Not Allowed";
    m_httpStatusMessage[406] = "Not Acceptable";
    m_httpStatusMessage[407] = "Proxy Authentication Required";
    m_httpStatusMessage[408] = "Request Timeout";
    m_httpStatusMessage[409] = "Conflict";
    m_httpStatusMessage[410] = "Gone";
    m_httpStatusMessage[411] = "Length Required";
    m_httpStatusMessage[412] = "Precondition Failed";
    m_httpStatusMessage[413] = "Request Entity Too Large";
    m_httpStatusMessage[414] = "Request-URI Too Long";
    m_httpStatusMessage[415] = "Unsupported Media Type";
    m_httpStatusMessage[416] = "Requested Range Not Satisfiable";
    m_httpStatusMessage[417] = "Expectation Failed";
    m_httpStatusMessage[422] = "Unprocessable Entity";
    m_httpStatusMessage[423] = "Locked";
    m_httpStatusMessage[424] = "Failed Dependency";
    m_httpStatusMessage[425] = "Unassigned";
    m_httpStatusMessage[426] = "Upgrade Required";
    m_httpStatusMessage[427] = "Unassigned";
    m_httpStatusMessage[428] = "Precondition Required";
    m_httpStatusMessage[429] = "Too Many Requests";
    m_httpStatusMessage[430] = "Unassigned";
    m_httpStatusMessage[431] = "Request Header Fields Too Large";
    m_httpStatusMessage[500] = "Internal Server Error";
    m_httpStatusMessage[501] = "Not Implemented";
    m_httpStatusMessage[502] = "Bad Gateway";
    m_httpStatusMessage[503] = "Service Unavailable";
    m_httpStatusMessage[504] = "Gateway Timeout";
    m_httpStatusMessage[505] = "HTTP Version Not Supported";
    m_httpStatusMessage[506] = "Variant Also Negotiates (Experimental)";
    m_httpStatusMessage[507] = "Insufficient Storage";
    m_httpStatusMessage[508] = "Loop Detected";
    m_httpStatusMessage[509] = "Unassigned";
    m_httpStatusMessage[510] = "Not Extended";
    m_httpStatusMessage[511] = "Network Authentication Required";
}

//-------------------------------------------------------------------------------------------------
ControllerAPIWebService::~ControllerAPIWebService()
{
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::operator()(Tools::WebServer::ConnectionContextPtr contextPtr)
{
    //contextPtr->getStat()->increment("total_queries", 1);

    pion::http::request_ptr request = contextPtr->getRequest();

    //const bool isGzipEnabled = (request->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    std::string action;
    ResourceParameters resourceParameters;
    if (!resourceParser().mapToAction(request->get_resource(), action, resourceParameters))
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST , "Can't parse resource url.");
        return;
    }

    try
    {

    }
    catch (const std::exception &e)
    {
    
    }

    //if (request->get_method() != pion::http::types::REQUEST_METHOD_POST
    //    && request->get_method() != pion::http::types::REQUEST_METHOD_GET)
    //{
    //    // bad request
    //    contextPtr->getStat()->increment("bad_queries", 1);

    //    const std::string badRequestData = std::string() +
    //        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" +
    //        "<document version=\"" + HttpCollector::Version::getMajor() + "." + HttpCollector::Version::getMinor() + "." + HttpCollector::Version::getPatch() + "\">\n" +
    //        "    <error>\n" +
    //        "        <type>bad request</type>\n" +
    //        "        <description>Only GET or POST method supported</description>\n" +
    //        "    </error>\n" +
    //        "</document>\n";

    //    pion::http::response_ptr responsePtr = createResponse(pion::http::types::RESPONSE_CODE_BAD_REQUEST,
    //        badRequestData,
    //        isGzipEnabled);

    //    contextPtr->sendResponse(responsePtr);
    //    return;
    //}

    //m_collectorPtr->asyncGetXmlResponse(request,
    //    boost::bind(&CollectorWebService::onCollectorResponse,
    //    this,
    //    contextPtr,
    //    _1),
    //    contextPtr->getTracer());

}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::start(void)
{
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::stop(void)
{
}

//-------------------------------------------------------------------------------------------------
const std::string &ControllerAPIWebService::getStatusMessage(unsigned statusCode) const
{
    std::map<unsigned, std::string>::const_iterator i = m_httpStatusMessage.find(statusCode);
    if (i == m_httpStatusMessage.end())
    {
        throw std::range_error("Can't find status message for code " + boost::lexical_cast<std::string>(statusCode));
    }
    return i->second;
}

//-------------------------------------------------------------------------------------------------
pion::http::response_ptr ControllerAPIWebService::createResponse(unsigned statusCode,
    const std::string &method,
    const std::string &contentType,
    const std::string &response,
    bool compress) const
{
    pion::http::response_ptr responsePtr(new pion::http::response(method));
    responsePtr->set_status_code(statusCode);
    responsePtr->set_status_message(getStatusMessage(statusCode));
    
    if (!response.empty())
        responsePtr->set_content_type(contentType);

    std::string responseBody;
    if (compress && !response.empty())
    {
        try
        {
            responseBody = Tools::CompressUtils::gzipStringCompress(response);
            responsePtr->add_header(pion::http::types::HEADER_CONTENT_ENCODING, "gzip");
        }
        catch (...)
        {
            responseBody = response;
        }
    }
    else
    {
        responseBody = response;
    }

    responsePtr->set_content_length(responseBody.length());
    responsePtr->set_content(responseBody);

    return responsePtr;
}

//-------------------------------------------------------------------------------------------------
bool ControllerAPIWebService::isStatusCodeValid(unsigned statusCode) const
{
    return m_httpStatusMessage.find(statusCode) != m_httpStatusMessage.end();
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::onControllerResponse(Tools::WebServer::ConnectionContextPtr contextPtr, 
    pion::http::response_ptr responsePtr)
{
    Tools::Logger::Logger &logger = Tools::Logger::Logger::getInstance();

    pion::http::request_ptr requestPtr = contextPtr->getRequest();
    const bool isGzipEnabled = (requestPtr->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    const std::string &url = requestPtr->get_resource();
    //if (!isStatusCodeValid(responsePtr->get_status_code()))
    //{
    //    logger.error(std::string("Processor returned unknown status code: ") + boost::lexical_cast<std::string>(responsePtr->get_status_code()));

    //    const std::string errorResponseData = std::string() +
    //        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" +
    //        "<document version=\"" + HttpCollector::Version::getMajor() + "." + HttpCollector::Version::getMinor() + "." + HttpCollector::Version::getPatch() + "\">\n" +
    //        "    <error>\n" +
    //        "        <type>internal error</type>\n" +
    //        "        <description>Processor returned unknown status code " + boost::lexical_cast<std::string>(responsePtr->get_status_code()) + "</description>\n" +
    //        "    </error>\n" +
    //        "</document>\n";
    //    responsePtr = createResponse(pion::http::types::RESPONSE_CODE_SERVER_ERROR, errorResponseData, isGzipEnabled);
    //}

    //if (responsePtr->get_content_length() == 0)
    //{
    //    logger.error("Processor returned empty content");

    //    const std::string errorResponseData = std::string() +
    //        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" +
    //        "<document version=\"" + HttpCollector::Version::getMajor() + "." + HttpCollector::Version::getMinor() + "." + HttpCollector::Version::getPatch() + "\">\n" +
    //        "    <error>\n" +
    //        "        <type>internal error</type>\n" +
    //        "        <description>Processor returned empty content" + "</description>\n" +
    //        "    </error>\n" +
    //        "</document>\n";
    //    responsePtr = createResponse(pion::http::types::RESPONSE_CODE_SERVER_ERROR, errorResponseData, isGzipEnabled);
    //}

    requestPtr->change_header(pion::http::types::HEADER_CONNECTION,
        contextPtr->getRequest()->check_keep_alive() ? "Keep-Alive" : "close");

    contextPtr->sendResponse(responsePtr);
}

//-------------------------------------------------------------------------------------------------
const ResourceParser &ControllerAPIWebService::resourceParser() const
{
    return m_parser;
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::sendErrorResponse(Tools::WebServer::ConnectionContextPtr contextPtr,
    unsigned statusCode,
    const std::string &errorMessage)
{
    const bool isGzipEnabled = (contextPtr->getRequest()->get_header("Accept-Encoding").find("gzip") != std::string::npos);
    
    Json::Value error(Json::objectValue);
    error["error"] = errorMessage;
    
    const std::string responseBody = Json::FastWriter().write(error);
    pion::http::response_ptr response = createResponse(statusCode, contextPtr->getRequest()->get_method(), "application/json", responseBody, isGzipEnabled);
    contextPtr->sendResponse(response);
}
