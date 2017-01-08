#pragma once

#include <map>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "Tools/Configuration/ConfigurationView.h"

#include "Process/ProcessConfiguration.h"

class Controller : private boost::noncopyable
{
public:
    explicit Controller(const Tools::Configuration::ConfigurationView &config);
    virtual ~Controller();

private:
    typedef std::map<std::string, ProcessConfiguration> ProcessConfigurations;
    
    ProcessConfigurations m_processConfigs;
};

typedef boost::shared_ptr<Controller> ControllerPtr;