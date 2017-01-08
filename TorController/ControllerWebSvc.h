#pragma once

#include <boost/shared_ptr.hpp>

#include "Tools/WebSvcApp/WebSvcApp.h"

#include "Controller.h"

class ControllerWebSvc :
    public Tools::WebSvcApp::WebSvcApp
{
public:
    ControllerWebSvc();
    virtual ~ControllerWebSvc();

private:
    void initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr) override;

    ControllerPtr m_controller;
};

