#pragma once

#include <map>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "Tools/Configuration/ConfigurationView.h"

#include "Options/AbstractCollection.h"
#include "Process/ProcessBase.h"
#include "Process/ProcessConfiguration.h"

class Controller : private boost::noncopyable
{
public:
    explicit Controller(const Tools::Configuration::ConfigurationView &config);
    virtual ~Controller();

    //void startProcess(const std::string &name);
    //void stopProcess(const std::string &name);
    //void getProcessState(const std::string &name);

    const Tools::Configuration::ConfigurationView &getConf() const;

private:
    Tools::Configuration::ConfigurationView m_config;

};

typedef boost::shared_ptr<Controller> ControllerPtr;