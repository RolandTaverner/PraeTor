#pragma once

#include <list>
#include <string>
#include <utility>

#include <boost/filesystem/path.hpp>
#include <boost/function/function1.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>

#include "Process/ProcessConfiguration.h"

typedef boost::function1<void, const boost::system::error_code &> ProcessActionHandler;

enum class ProcessState
{
  Stopped,
  Initializing,
  Running
};

class IProcess
{
public:
    virtual ~IProcess() {}

    virtual const std::string &name() const = 0;

    virtual const std::string &executable() const = 0;

    virtual const boost::filesystem::path &rootPath() const = 0;

    virtual const ProcessConfiguration &getConfiguration() const = 0;

    virtual void start(const ProcessActionHandler &handler) = 0;

    virtual std::string cmdLineConfigName() const = 0;

    virtual std::string fileConfigName() const = 0;

	virtual bool isRunning() const = 0;

	virtual bool hasConfigFile() const = 0;

    virtual void getConfigurations(std::list<std::string> &configs) const = 0;

    virtual void getConfigurationOptions(const std::string &configName, std::list<std::string> &options) const = 0;

    virtual OptionDesc getOptionDesc(const std::string &configName, const std::string &optionName) const = 0;

    virtual OptionDescValue getOptionValue(const std::string &configName, const std::string &optionName) const = 0;
};

typedef boost::shared_ptr<IProcess> IProcessPtr;