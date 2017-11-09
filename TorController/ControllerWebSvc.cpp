#include "WebServices/ControllerAPIWebService.h"
#include "ControllerWebSvc.h"
#include "gen/Version.h"

//-------------------------------------------------------------------------------------------------
ControllerWebSvc::ControllerWebSvc() : Tools::WebSvcApp::WebSvcApp(PRODUCT_FULLNAME_STRING, PRODUCT_VERSION_STRING, "0")
{
}

//-------------------------------------------------------------------------------------------------
ControllerWebSvc::~ControllerWebSvc()
{
}

//-------------------------------------------------------------------------------------------------
void ControllerWebSvc::initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr)
{
    m_controller.reset(new Controller(getConf().branch("serviceconfig.controller")));

    webServiceRegistrar.registerService("/api/controller", Tools::WebServer::IWebServicePtr(new ControllerAPIWebService(m_controller)));
}