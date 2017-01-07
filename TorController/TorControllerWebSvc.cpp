#include "TorControllerWebSvc.h"

//-------------------------------------------------------------------------------------------------
TorControllerWebSvc::TorControllerWebSvc() : Tools::WebSvcApp::WebSvcApp("EasyTor", "1.0.0", "0")
{
}

//-------------------------------------------------------------------------------------------------
TorControllerWebSvc::~TorControllerWebSvc()
{
}

//-------------------------------------------------------------------------------------------------
void TorControllerWebSvc::initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr)
{

}