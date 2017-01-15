#include <boost/foreach.hpp>

#include "Options/ConfigScheme.h"
#include "Options/OptionsStorage.h"
#include "Process/ProcessBase.h"

//-------------------------------------------------------------------------------------------------
ProcessBase::ProcessBase(const Tools::Configuration::ConfigurationView &conf)
{
    m_name = conf.getAttr("", "name");
    m_executable = conf.get("executable");
    m_rootPath = conf.get("root");
    
    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &schemeConf, conf.getRangeOf("options.scheme"))
    {
        const std::string name = schemeConf.getAttr("", "name");
        IConfigSchemePtr scheme = ConfigScheme::CreateFromConfig(schemeConf);
        IOptionsStoragePtr storage(new OptionsStorage(scheme));
        m_configuration.addStorage(name, storage);
    }
}

//-------------------------------------------------------------------------------------------------
ProcessBase::~ProcessBase()
{
}

//-------------------------------------------------------------------------------------------------
const std::string &ProcessBase::name() const
{
    return m_name;
}

//-------------------------------------------------------------------------------------------------
const std::string &ProcessBase::executable() const
{
    return m_executable;
}

//-------------------------------------------------------------------------------------------------
const boost::filesystem::path &ProcessBase::rootPath() const
{
    return m_rootPath;
}

//-------------------------------------------------------------------------------------------------
const ProcessConfiguration &ProcessBase::getConfiguration() const
{
    return m_configuration;
}