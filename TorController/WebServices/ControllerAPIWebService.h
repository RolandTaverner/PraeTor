#pragma once

#include <boost/function/function2.hpp>

#include "Tools/WebServer/IWebService.h"

#include "Controller/Controller.h"
#include "WebServices/ResourceParser.h"
#include "Error.h"

enum class ResourceActionType { Get, Put, Delete };

class ControllerAPIWebService :
    public Tools::WebServer::IWebService
{
public:
    explicit ControllerAPIWebService(ControllerPtr controller);
    virtual ~ControllerAPIWebService();

    // Tools::WebServer::IWebService implementation
    void operator()(Tools::WebServer::ConnectionContextPtr contextPtr) override;
    
    void start(void) override;
    
    void stop(void) override;

private:
    ControllerPtr m_controller;
    // Helpers
    std::map<unsigned, std::string> m_httpStatusMessage;

    const std::string &getStatusMessage(unsigned statusCode) const;

    bool isStatusCodeValid(unsigned statusCode) const;

    pion::http::response_ptr createResponse(unsigned statusCode, 
        const std::string &method,
        const std::string &contentType,
        const std::string &response, 
        bool compress) const;

    void sendResponse(Tools::WebServer::ConnectionContextPtr contextPtr, const std::string &message);

    void sendErrorResponse(Tools::WebServer::ConnectionContextPtr contextPtr,
        unsigned statusCode,
        const std::string &errorMessage);

    void sendErrorResponse(Tools::WebServer::ConnectionContextPtr contextPtr, const ErrorCode &ec);

    ResourceParser m_parser;
    const ResourceParser &resourceParser() const;

     // Actions
    typedef boost::function2<void, Tools::WebServer::ConnectionContextPtr, const ResourceParameters &> ActionHandler;
    typedef std::map<std::string, ActionHandler> ActionHandlers;
    ActionHandlers m_handlers;

    void controllerInfoAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void presetsAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void getPresetsGroupAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processesAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processInfoAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processConfigsAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processConfigAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processOptionAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);
    void onProcessOptionResponse(Tools::WebServer::ConnectionContextPtr contextPtr, ResourceActionType actionType, const ProcessOptionResult &result);

    void processAction(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void processLog(Tools::WebServer::ConnectionContextPtr contextPtr, const ResourceParameters &parameters);

    void defaultResponseHandler(Tools::WebServer::ConnectionContextPtr contextPtr, const ActionResult &result);
};
