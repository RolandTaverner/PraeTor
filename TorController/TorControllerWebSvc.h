#pragma once

#include "Tools/WebSvcApp/WebSvcApp.h"

class TorControllerWebSvc :
    public Tools::WebSvcApp::WebSvcApp
{
public:
    TorControllerWebSvc();
    virtual ~TorControllerWebSvc();

private:
    void initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr) override;
};

