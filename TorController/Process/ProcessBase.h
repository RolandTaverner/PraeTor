#pragma once

#include <string>

#include <boost/shared_ptr.hpp>

#include "Options/IConfigScheme.h"
#include "Options/IOptionsStorage.h"
#include "Process/IProcess.h"
#include "Process/ProcessConfiguration.h"

#include "Tools/Configuration/ConfigurationView.h"

class ProcessBase : public IProcess
{
public:
    explicit ProcessBase(const Tools::Configuration::ConfigurationView &conf);
    virtual ~ProcessBase();

    // IProcess
    const std::string &name() const override;

    const std::string &executable() const override;

    const boost::filesystem::path &rootPath() const override;

private:
    std::string m_name;
    std::string m_executable;
    boost::filesystem::path m_rootPath;

    ProcessConfiguration m_configuration;
};

typedef boost::shared_ptr<ProcessBase> ProcessBasePtr;