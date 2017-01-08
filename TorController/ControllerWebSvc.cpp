#include "ControllerWebSvc.h"

//-------------------------------------------------------------------------------------------------
ControllerWebSvc::ControllerWebSvc() : Tools::WebSvcApp::WebSvcApp("EasyTor", "1.0.0", "0")
{
}

//-------------------------------------------------------------------------------------------------
ControllerWebSvc::~ControllerWebSvc()
{
}

//-------------------------------------------------------------------------------------------------
void ControllerWebSvc::initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr)
{
    m_controller.reset(new Controller(getConf().branch("controller")));
}