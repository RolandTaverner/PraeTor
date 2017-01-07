// C++
#include <stdexcept>

#include "Tools/WebServer/ConfService.h"
#include "Tools/WebServer/Errors.h"
#include "Tools/CompressUtils/Compress.h"

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
ConfService::ConfService(boost::function<std::pair<std::string,std::string>()> confCallback)
: m_confCallback(confCallback)
{
    if (!m_confCallback)
    {
        throw WebServerError("Empty configuration callback!");
    }
}

//--------------------------------------------------------------------------------------------------
ConfService::~ConfService()
{
}

//--------------------------------------------------------------------------------------------------
void ConfService::start()
{
}

//--------------------------------------------------------------------------------------------------
void ConfService::stop()
{
}

//--------------------------------------------------------------------------------------------------
void ConfService::operator()(ConnectionContextPtr contextPtr)
{
    pion::http::request_ptr requestPtr = contextPtr->getRequest();

    const std::string method = requestPtr->get_method();
//    const std::string url = requestPtr->get_resource();
//    const pion::ihash_multimap &params = requestPtr->get_queries();

    const bool isGzipEnabled = (requestPtr->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    if (method != pion::http::types::REQUEST_METHOD_GET)
    {
        // bad request
        static const std::string badRequestData = "Bad request. Request method must be GET.";

        pion::http::response_ptr responsePtr(new pion::http::response(pion::http::types::REQUEST_METHOD_GET));
        responsePtr->set_status_code(pion::http::types::RESPONSE_CODE_BAD_REQUEST);
        responsePtr->set_status_message(pion::http::types::RESPONSE_MESSAGE_BAD_REQUEST);
        responsePtr->set_content_type("text/plain;charset=UTF-8");
        responsePtr->add_header(pion::http::types::HEADER_CONNECTION, "close");

        std::string response = badRequestData;
        if (isGzipEnabled)
        {
            try
            {
                response = Tools::CompressUtils::gzipStringCompress(badRequestData);
                responsePtr->add_header(pion::http::types::HEADER_CONTENT_ENCODING, "gzip");
            }
            catch (...)
            {
                response = badRequestData;
            }
        }

        responsePtr->set_content_length(response.length());
        responsePtr->set_content(response);

        contextPtr->sendResponse(responsePtr);
        return;
    }

    const std::pair<std::string,std::string> &confWithContentType = m_confCallback();

    const std::string &conf = confWithContentType.first;
    const std::string &contentType = confWithContentType.second;

    pion::http::response_ptr responsePtr(new pion::http::response(pion::http::types::REQUEST_METHOD_GET));
    responsePtr->set_status_code(pion::http::types::RESPONSE_CODE_OK);
    responsePtr->set_status_message(pion::http::types::RESPONSE_MESSAGE_OK);
    responsePtr->set_content_type(contentType);

     if (isGzipEnabled)
     {
         std::string compressedResponse;
         try
         {
             compressedResponse = Tools::CompressUtils::gzipStringCompress(conf);
             responsePtr->add_header(pion::http::types::HEADER_CONTENT_ENCODING, "gzip");
         }
         catch(...)
         {
             compressedResponse = conf;
         }
         responsePtr->set_content_length(compressedResponse.length());
         responsePtr->set_content(compressedResponse);
     }
     else
     {
         responsePtr->set_content_length(conf.length());
         responsePtr->set_content(conf);
     }

     contextPtr->sendResponse(responsePtr);
}

} /* namespace WebServer */
} /* namespace Tools */
