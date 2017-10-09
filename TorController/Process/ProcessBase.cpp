#include <fstream>

#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/process.hpp>

#include "Options/ConfigScheme.h"
#include "Options/OptionsStorage.h"
#include "Process/ProcessBase.h"
#include "Process/ProcessErrors.h"

namespace bp = boost::process;

const char *ProcessBase::s_configFileSection = "config";
const char *ProcessBase::s_cmdLineSection = "cmdline";

//-------------------------------------------------------------------------------------------------
ProcessBase::ProcessBase(const Tools::Configuration::ConfigurationView &conf, pion::scheduler &scheduler) :
    m_scheduler(scheduler)
{
    m_name = conf.getAttr("", "name");
    m_executable = conf.get("executable");
    m_rootPath = conf.get("root");
    
    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &schemeConf, conf.getRangeOf("options.scheme"))
    {
        const std::string name = schemeConf.getAttr("", "name");
        IConfigSchemePtr scheme = ConfigScheme::createFromConfig(schemeConf);
        IOptionsStoragePtr storage(new OptionsStorage(scheme));
        m_configuration.addStorage(name, storage);
    }

    m_scheduler.add_active_user();
}

//-------------------------------------------------------------------------------------------------
ProcessBase::~ProcessBase()
{
    m_scheduler.remove_active_user();
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

//-------------------------------------------------------------------------------------------------
void ProcessBase::start(const ProcessActionHandler &handler)
{
	UniqueLockType lock(m_access);

	if (isRunningInternal()) {
		throw ProcessError(makeErrorCondition(ProcessErrors::alreadyRunning));
	}

	if (hasConfigFile())
	{

	  // create config
	}

	const std::string exePath = (rootPath() / executable()).string();
	const std::string args;
	
	m_childPtr.reset(new bp::child(bp::exe = exePath, bp::args = args));

}


//-------------------------------------------------------------------------------------------------
std::string ProcessBase::cmdLineConfigName() const
{
    return s_cmdLineSection;
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::fileConfigName() const
{
    return s_configFileSection;
}

//-------------------------------------------------------------------------------------------------
bool ProcessBase::isRunning() const
{
	SharedLockType lock(m_access);

	return isRunningInternal();
}

//-------------------------------------------------------------------------------------------------
bool ProcessBase::isRunningInternal() const
{
	return m_childPtr && m_childPtr->running();
}

//-------------------------------------------------------------------------------------------------
bool ProcessBase::hasConfigFile() const
{
	return m_configuration.hasStorage(s_configFileSection);
}

//-------------------------------------------------------------------------------------------------
boost::filesystem::path ProcessBase::createConfigFile()
{
    BOOST_ASSERT(hasConfigFile());
    //TODO: implement
    
    boost::filesystem::path outFilePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    std::ofstream outFile(outFilePath.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!outFile)
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::configFileWriteError), "Can't create file " + outFilePath.string());
    }

    IOptionsStoragePtr configPtr = m_configuration.getStorage(s_configFileSection);
    IConfigSchemePtr schemePtr = configPtr->getScheme();

    BOOST_FOREACH(const Option &o, configPtr->getRange())
    {
    
    }
    

    return m_configFilePath;
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::getConfigurations(std::list<std::string> &configs) const
{
    SharedLockType lock(m_access);
    m_configuration.getStorages(configs);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::getConfigurationOptions(const std::string &configName, std::list<std::string> &options) const
{
    SharedLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::noSuchStorage),
                           "Cant't find storage " + configName + " in process " + name());
    }

    IOptionsStorageConstPtr storagePtr = m_configuration.getStorage(configName);
    for (const OptionDesc &o : storagePtr->getScheme()->getRange())
    {
        options.push_back(o.get<0>());
    }
}

//-------------------------------------------------------------------------------------------------
OptionDesc ProcessBase::getOptionDesc(const std::string &configName, const std::string &optionName) const
{
    SharedLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::noSuchStorage),
                           "Cant't find storage " + configName + " in process " + name());
    }

    IOptionsStorageConstPtr storagePtr = m_configuration.getStorage(configName);

    if (!storagePtr->getScheme()->hasOption(optionName))
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::noSuchOption),
                           "Cant't find option " + optionName + " in storage " + configName + " in process " + name());
    }

    return storagePtr->getScheme()->getOptionDesc(optionName);
}

//-------------------------------------------------------------------------------------------------
OptionDescValue ProcessBase::getOptionValue(const std::string &configName, const std::string &optionName) const
{
    SharedLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::noSuchStorage),
            "Cant't find storage " + configName + " in process " + name());
    }

    IOptionsStorageConstPtr storagePtr = m_configuration.getStorage(configName);
    IConfigSchemePtr schemePtr = storagePtr->getScheme();

    if (!schemePtr->hasOption(optionName))
    {
        throw ProcessError(makeErrorCondition(ProcessErrors::noSuchOption),
            "Cant't find option " + optionName + " in storage " + configName + " in process " + name());
    }

    OptionDescValue result;
    const OptionDesc desc = storagePtr->getScheme()->getOptionDesc(optionName);
    const OptionValueType value = storagePtr->hasValue(optionName) ? storagePtr->getValue(optionName) :
        (schemePtr->hasDefaultValue(optionName) ? schemePtr->getDefaultValue(optionName) : OptionValueType());
    const std::string presentation = storagePtr->hasValue(optionName) ? storagePtr->formatOption(optionName) : "";
    return OptionDescValue(desc, value, presentation);
}

//-------------------------------------------------------------------------------------------------
ProcessState ProcessBase::getState() const
{
    return m_state;
}