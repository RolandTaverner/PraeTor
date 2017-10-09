#pragma once

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "pion/scheduler.hpp"

#include "Options/IConfigScheme.h"
#include "Options/IOptionsStorage.h"
#include "Process/IProcess.h"
#include "Process/ProcessConfiguration.h"

#include "Tools/Configuration/ConfigurationView.h"

namespace boost
{
namespace process
{
class child;
}
}

typedef boost::shared_ptr<boost::process::child> ChildProcessPtr;

class ProcessBase : 
	public IProcess, 
	public boost::enable_shared_from_this<ProcessBase>,
	private boost::noncopyable
{
public:
    ProcessBase(const Tools::Configuration::ConfigurationView &conf, pion::scheduler &scheduler);
    virtual ~ProcessBase();

    // IProcess
    const std::string &name() const override;

    const std::string &executable() const override;

    const boost::filesystem::path &rootPath() const override;

    const ProcessConfiguration &getConfiguration() const override;

    void start(const ProcessActionHandler &handler) override;

    std::string cmdLineConfigName() const override;

    std::string fileConfigName() const override;
	
    bool isRunning() const override;

	bool hasConfigFile() const override;

    void getConfigurations(std::list<std::string> &configs) const override;

    void getConfigurationOptions(const std::string &configName, std::list<std::string> &options) const override;

    OptionDesc getOptionDesc(const std::string &configName, const std::string &optionName) const override;

    OptionDescValue getOptionValue(const std::string &configName, const std::string &optionName) const override;

    ProcessState getState() const override;

	static const char *s_configFileSection;
    static const char *s_cmdLineSection;

private:
    
	bool isRunningInternal() const;

	boost::filesystem::path createConfigFile();

		
    pion::scheduler &m_scheduler;
    std::string m_name;
    std::string m_executable;
    boost::filesystem::path m_rootPath;
    ProcessConfiguration m_configuration;
    volatile ProcessState m_state;

	boost::filesystem::path m_configFilePath;
	ChildProcessPtr m_childPtr;

	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock_guard<MutexType> SharedLockType;
	typedef boost::lock_guard<MutexType> UniqueLockType;
	
	mutable MutexType m_access;
};

typedef boost::shared_ptr<ProcessBase> ProcessBasePtr;