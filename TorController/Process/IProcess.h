#pragma once

#include <list>
#include <string>
#include <utility>

#include <boost/filesystem/path.hpp>
#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>
#include <boost/shared_ptr.hpp>

#include "Process/ProcessConfiguration.h"
#include "Error.h"

enum class ProcessState
{
    Starting = 1,
    Running = 2,
    Stopping = 3,
    Stopped = 4
};

struct ExitStatus
{
    ExitStatus() : exitCode(0), unexpectedExit(false) {}

    ExitStatus(int _exitCode, const std::error_code &_errorCode, bool _unexpectedExit) :
        exitCode(_exitCode),
        errorCode(_errorCode),
        unexpectedExit(_unexpectedExit) {}
    
    int exitCode;
    std::error_code errorCode;
    bool unexpectedExit;
};

typedef boost::function1<void, const ErrorCode &> StartProcessHandler;
typedef boost::function2<void, const ErrorCode &, const ExitStatus &> StopProcessHandler;
typedef boost::function1<void, const std::string &> LogLineHandler;


class IProcess
{
public:
    virtual ~IProcess() {}

    virtual const std::string &name() const = 0;

    virtual const std::string &executable() const = 0;

    virtual const boost::filesystem::path &rootPath() const = 0;
    
    virtual const boost::filesystem::path &dataRootPath() const = 0;

    virtual const ProcessConfiguration &getConfiguration() const = 0;

    virtual void start(const StartProcessHandler &handler) = 0;

    virtual void stop(const StopProcessHandler &handler) = 0;

    virtual std::string cmdLineConfigName() const = 0;

    virtual std::string fileConfigName() const = 0;

	virtual bool isRunning() const = 0;

	virtual bool hasConfigFile() const = 0;

    virtual void getConfigurations(std::list<std::string> &configs) const = 0;

    virtual void getConfigurationOptions(const std::string &configName, std::list<std::string> &options) const = 0;

    virtual OptionDesc getOptionDesc(const std::string &configName, const std::string &optionName) const = 0;

    virtual OptionDescValue getOptionValue(const std::string &configName, const std::string &optionName) const = 0;

    virtual OptionDescValue setOptionValue(const std::string &configName,
        const std::string &optionName,
        const OptionValueContainer &optionValue) = 0;

    virtual ProcessState getState() const = 0;

    virtual ExitStatus getExitStatus() const = 0;

    virtual void applyConfig(const ProcessConfiguration &presetConf) = 0;

    virtual void applyUserConfig(const ProcessConfiguration &presetConf) = 0;

    virtual void getLog(LogLineHandler &acc) = 0;
};

typedef boost::shared_ptr<IProcess> IProcessPtr;