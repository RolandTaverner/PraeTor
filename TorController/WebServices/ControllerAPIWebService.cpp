#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <pion/algorithm.hpp>

#include <json/value.h>
#include <json/writer.h>
#include <json/reader.h>

#include "Tools/CompressUtils/Compress.h"
#include "Tools/Logger/Logger.h"
#include "Tools/StringUtils/StringEscapeUtils.h"
#include "Tools/StringUtils/UnicodeTextProcessing.h"

#include "Controller/ControllerErrors.h"
#include "WebServices/ControllerAPIWebService.h"
#include "WebServices/ResourceActions.h"

static const char *s_resourceScheme =
"{"
"  \"node\": \"api\","
"  \"next\":"
"  ["
"    {"
"      \"node\" : \"controller\","
"      \"action\":  \"ControllerInfo\","
"      \"next\" :"
"      ["
"        {"
"          \"node\" : \"presets\","
"          \"action\":  \"Presets\","
"          \"next\" :"
"          ["
"            {"
"              \"node\" : \"$preset_name\","
"              \"action\":  \"GetPresetsGroup\""
"            }"
"          ]"
"        },"
"        {"
"          \"node\" : \"processes\","
"          \"action\":  \"Processes\","
"          \"next\" :"
"          ["
"            { "
"              \"node\" : \"$process_id\","
"              \"action\":  \"ProcessInfo\","
"              \"next\" :"
"              ["
"                { "
"                  \"node\" : \"action\","
"                  \"action\":  \"ProcessAction\""
"                },"
"                { "
"                  \"node\" : \"log\","
"                  \"action\":  \"ProcessLog\""
"                },"
"                { "
"                  \"node\" : \"configs\","
"                  \"action\":  \"ProcessConfigs\","
"                  \"next\" :"
"                  ["
"                    {"
"                      \"node\" : \"$config_name\","
"                      \"action\" : \"ProcessConfig\","
"                      \"next\" :"
"                      ["
"                        {"
"                          \"node\" : \"options\","
"                          \"next\" :"
"                          ["
"                            {"
"                              \"node\" : \"$option_name\","
"                              \"action\" : \"ProcessOption\""
"                            }"
"                          ]"
"                        }"
"                      ]"
"                    }"
"                  ]"
"                }"
"              ]"
"            }"
"          ]"
"        }"
"      ]"
"    }"
"  ]"
"}";

//-------------------------------------------------------------------------------------------------
ControllerAPIWebService::ControllerAPIWebService(ControllerPtr controller) :
    m_controller(controller), m_parser(s_resourceScheme)
{
    m_handlers[ResourceActions::nodeControllerInfo] = boost::bind(&ControllerAPIWebService::controllerInfoAction, this, _1, ResourceActions::nodeControllerInfo, _2);
    m_handlers[ResourceActions::nodePresets] = boost::bind(&ControllerAPIWebService::presetsAction, this, _1, ResourceActions::nodePresets, _2);
    m_handlers[ResourceActions::nodeGetPresetsGroup] = boost::bind(&ControllerAPIWebService::getPresetsGroupAction, this, _1, ResourceActions::nodeGetPresetsGroup, _2);
    m_handlers[ResourceActions::nodeProcesses] = boost::bind(&ControllerAPIWebService::processesAction, this, _1, ResourceActions::nodeProcesses, _2);
    m_handlers[ResourceActions::nodeProcessInfo] = boost::bind(&ControllerAPIWebService::processInfoAction, this, _1, ResourceActions::nodeProcessInfo, _2);
    m_handlers[ResourceActions::nodeProcessConfigs] = boost::bind(&ControllerAPIWebService::processConfigsAction, this, _1, ResourceActions::nodeProcessConfigs,  _2);
    m_handlers[ResourceActions::nodeProcessConfig] = boost::bind(&ControllerAPIWebService::processConfigAction, this, _1, ResourceActions::nodeProcessConfig, _2);
    m_handlers[ResourceActions::nodeProcessOption] = boost::bind(&ControllerAPIWebService::processOptionAction, this, _1, ResourceActions::nodeProcessOption, _2);
    m_handlers[ResourceActions::nodeProcessAction] = boost::bind(&ControllerAPIWebService::processAction, this, _1, ResourceActions::nodeProcessAction, _2);
    m_handlers[ResourceActions::nodeProcessLog] = boost::bind(&ControllerAPIWebService::processLog, this, _1, ResourceActions::nodeProcessLog, _2);

    // HTTP status messages
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
}

//-------------------------------------------------------------------------------------------------
ControllerAPIWebService::~ControllerAPIWebService()
{
    m_controller->stop();
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::operator()(Tools::WebServer::ConnectionContextPtr contextPtr)
{
    //contextPtr->getStat()->increment("total_queries", 1);

    try
    {
        if (contextPtr->getRequest()->get_method() == "OPTIONS")
        {
            pion::http::response_ptr responsePtr = createResponse(pion::http::types::RESPONSE_CODE_OK, "OPTIONS", "text/plain", "", false);

            responsePtr->add_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
            
            std::ostringstream h;
            h << pion::http::types::HEADER_CONTENT_TYPE << ", ";
            h << pion::http::types::HEADER_HOST << ", ";
            h << pion::http::types::HEADER_CONNECTION << ", ";
            h << pion::http::types::HEADER_CONTENT_LENGTH << ", ";
            h << pion::http::types::HEADER_CONTENT_ENCODING << ", ";
            h << pion::http::types::HEADER_USER_AGENT << ", ";
            
            responsePtr->add_header("Access-Control-Allow-Headers",  h.str());
            contextPtr->sendResponse(responsePtr);
            return;
        }
        
        ResourceParameters resourceParameters;

        const std::string action = resourceParser().mapToAction(contextPtr->getRequest()->get_resource(), resourceParameters);

        ActionHandlers::const_iterator it = m_handlers.find(action);
        if (it == m_handlers.end())
        {
            sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Action not found.");
            return;
        }

        (it->second)(contextPtr, resourceParameters);
    }
    catch (const ResourceParserError &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, std::string("Can't parse resource url: ") + e.what());
        return;
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, std::string("Internal error: ") + e.what());
        return;
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::start(void)
{
    m_controller->start();
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::stop(void)
{
    m_controller->stop();
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
    
    responsePtr->add_header("Access-Control-Allow-Origin", "*");
    
    return responsePtr;
}

//-------------------------------------------------------------------------------------------------
bool ControllerAPIWebService::isStatusCodeValid(unsigned statusCode) const
{
    return m_httpStatusMessage.find(statusCode) != m_httpStatusMessage.end();
}

//-------------------------------------------------------------------------------------------------
const ResourceParser &ControllerAPIWebService::resourceParser() const
{
    return m_parser;
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::sendResponse(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &message)
{
    const bool isGzipEnabled = (contextPtr->getRequest()->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    pion::http::response_ptr response = createResponse(pion::http::types::RESPONSE_CODE_OK,
        contextPtr->getRequest()->get_method(),
        "application/json;charset=utf-8",
        message,
        isGzipEnabled);

    contextPtr->sendResponse(response);
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

    response->change_header(pion::http::types::HEADER_CONNECTION,
        contextPtr->getRequest()->check_keep_alive() ? "Keep-Alive" : "close");

    contextPtr->sendResponse(response);
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::controllerInfoAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    try
    {
        m_controller->getControllerInfo(boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processesAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    try
    {
        m_controller->getProcesses(boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processInfoAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    const ResourceParameters::const_iterator i = parameters.find("process_id");
    if (i == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    try
    {
        m_controller->getProcessInfo(i->second, boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processConfigsAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    const ResourceParameters::const_iterator i = parameters.find("process_id");
    if (i == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    try
    {
        m_controller->getProcessConfigs(i->second, boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processConfigAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    const ResourceParameters::const_iterator itProcessId = parameters.find("process_id");
    const ResourceParameters::const_iterator itConfigName = parameters.find("config_name");
    if (itProcessId == parameters.end() || itConfigName == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    try
    {
        m_controller->getProcessConfig(itProcessId->second, 
                                       itConfigName->second,
                                       boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processOptionAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    const ResourceParameters::const_iterator itProcessId = parameters.find("process_id");
    const ResourceParameters::const_iterator itConfigName = parameters.find("config_name");
    const ResourceParameters::const_iterator itOptionName = parameters.find("option_name");

    if (itProcessId == parameters.end() || itConfigName == parameters.end() || itOptionName == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    try
    {
        if (contextPtr->getRequest()->get_method() == pion::http::types::REQUEST_METHOD_GET)
        {
            m_controller->getProcessOption(itProcessId->second,
                itConfigName->second,
                itOptionName->second,
                boost::bind(&ControllerAPIWebService::onProcessOptionResponse, this, contextPtr, action, ResourceActionType::Get, _1));
        }
        else if (contextPtr->getRequest()->get_method() == pion::http::types::REQUEST_METHOD_PUT)
        {
            Json::Value v;
            if (!Json::Reader().parse(contextPtr->getRequest()->get_content(), v, false) 
                || (v["value"].isNull() && v["array_value"].isNull()))
            {
                sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "Can't parse set process option request content.");
                return;
            }

            if (!v["value"].isNull() && !v["array_value"].isNull())
            {
                sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "Process option value can't has both value and array_value fields.");
                return;
            }

            OptionValueContainer newOptionValue;
            if (!v["value"].isNull())
            {
                newOptionValue = OptionSingleValue(v["value"].asString());
            }
            else
            {
                OptionListValue listValue;
                Json::Value arrayValue = v["array_value"];
                if (!arrayValue.isArray())
                {
                    sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "array_value field should be array of strings or numbers.");
                    return;
                }
                
                for (Json::ArrayIndex i = 0; i < arrayValue.size(); ++i)
                {
                    const Json::Value &item = arrayValue[i];
                    if (item.isNumeric() || item.isString())
                    {
                        listValue.push_back(item.asString());
                    }
                    else
                    {
                        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "array_value field should be array of strings or numbers.");
                        return;
                    }
                }
                newOptionValue = listValue;
            }

            m_controller->setProcessOption(itProcessId->second,
                itConfigName->second,
                itOptionName->second,
                newOptionValue,
                boost::bind(&ControllerAPIWebService::onProcessOptionResponse, this, contextPtr, action, ResourceActionType::Put, _1));
        }
        else if (contextPtr->getRequest()->get_method() == pion::http::types::REQUEST_METHOD_DELETE)
        {
            m_controller->removeProcessOption(itProcessId->second,
                itConfigName->second,
                itOptionName->second,
                boost::bind(&ControllerAPIWebService::onProcessOptionResponse, this, contextPtr, action, ResourceActionType::Delete, _1));
        }
        else
        {
            sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        }

    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::onProcessOptionResponse(Tools::WebServer::ConnectionContextPtr contextPtr,
    const std::string &action,
    ResourceActionType actionType,
    const ProcessOptionResult &result)
{
    if (result.getError())
    {
        sendErrorResponse(contextPtr, m_errorsMapping.getHttpStatusCode(action, contextPtr->getRequest()->get_method(), result.getError()), result.getError());
        return;
    }

    sendResponse(contextPtr, result.toJson().toStyledString());
}
//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    const ResourceParameters::const_iterator itProcessId = parameters.find("process_id");

    if (itProcessId == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_POST)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Only POST method allowed.");
        return;
    }

    Json::Value v;
    if (!Json::Reader().parse(contextPtr->getRequest()->get_content(), v, false) || v["action"].isNull())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "Can't parse process action request content.");
        return;
    }

    const Json::Value &processActionName = v["action"];
    if (boost::algorithm::equals(processActionName.asString(), "start"))
    {
        m_controller->startProcess(itProcessId->second,
            boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
        return;
    }
    else if (boost::algorithm::equals(processActionName.asString(), "stop"))
    {
        m_controller->stopProcess(itProcessId->second,
            boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
        return;
    }

    sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "Unknown process action requested.");
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::sendErrorResponse(Tools::WebServer::ConnectionContextPtr contextPtr, unsigned httpStatusCode, const ErrorCode &ec)
{
    const bool isGzipEnabled = (contextPtr->getRequest()->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    Json::Value error(Json::objectValue);
    error["error"] = ec.message();
    error["category"] = ec.category();
    error["value"] = ec.value();

    const std::string responseBody = Json::FastWriter().write(error);

    pion::http::response_ptr response = createResponse(httpStatusCode,
        contextPtr->getRequest()->get_method(), 
        "application/json", 
        responseBody, 
        isGzipEnabled);

    response->change_header(pion::http::types::HEADER_CONNECTION,
        contextPtr->getRequest()->check_keep_alive() ? "Keep-Alive" : "close");

    contextPtr->sendResponse(response);
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::defaultResponseHandler(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ActionResult &result)
{
    if (result.getError())
    {
        sendErrorResponse(contextPtr, m_errorsMapping.getHttpStatusCode(action, contextPtr->getRequest()->get_method(), result.getError()), result.getError());
        return;
    }

    sendResponse(contextPtr, result.toJson().toStyledString());
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::presetsAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() == pion::http::types::REQUEST_METHOD_GET)
    {
        try
        {
            m_controller->getPresetGroups(boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
        }
        catch (const std::exception &e)
        {
            sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
        }
    }
    else if (contextPtr->getRequest()->get_method() == pion::http::types::REQUEST_METHOD_POST)
    {
        try
        {
            Json::Value v;
            if (!Json::Reader().parse(contextPtr->getRequest()->get_content(), v, false) 
                || v["name"].isNull() 
                || !v["name"].isString())
            {
                sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_BAD_REQUEST, "Can't parse process action request content.");
                return;
            }

            const std::string name = v["name"].asString();

            m_controller->applyPresetGroup(name, boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
        }
        catch (const std::exception &e)
        {
            sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
        }
    }
    else
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::getPresetsGroupAction(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    const ResourceParameters::const_iterator itPresetGroupName = parameters.find("preset_name");

    if (itPresetGroupName == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    const std::string &name = itPresetGroupName->second;

    try
    {
        m_controller->getPresets(name, boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}

//-------------------------------------------------------------------------------------------------
void ControllerAPIWebService::processLog(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &action, const ResourceParameters &parameters)
{
    if (contextPtr->getRequest()->get_method() != pion::http::types::REQUEST_METHOD_GET)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_METHOD_NOT_ALLOWED, "Method " + contextPtr->getRequest()->get_method() + " not allowed.");
        return;
    }

    const ResourceParameters::const_iterator i = parameters.find("process_id");
    if (i == parameters.end())
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, "Something went wrong with resource parser.");
        return;
    }

    try
    {
        m_controller->getProcessLog(i->second, boost::bind(&ControllerAPIWebService::defaultResponseHandler, this, contextPtr, action, _1));
    }
    catch (const std::exception &e)
    {
        sendErrorResponse(contextPtr, pion::http::types::RESPONSE_CODE_SERVER_ERROR, e.what());
    }
}