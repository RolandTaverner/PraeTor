#include <windows.h>

#include <fstream>

#include <boost/algorithm/string/compare.hpp>
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/function/function0.hpp>
#include <boost/function/function1.hpp>
#include <boost/function/function2.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/process.hpp>
#include <boost/process/extend.hpp>

#include "Tools/Logger/Logger.h"

#include "Options/ConfigScheme.h"
#include "Options/OptionsStorage.h"
#include "Process/ProcessBase.h"
#include "Process/ProcessErrors.h"

namespace fs = boost::filesystem;
namespace bp = boost::process;
namespace ex = bp::extend;

const char *ProcessBase::s_configFileSection = "config";
const char *ProcessBase::s_cmdLineSection = "cmdline";

//-------------------------------------------------------------------------------------------------
ProcessBase::ProcessBase(const Tools::Configuration::ConfigurationView &conf, pion::scheduler &scheduler) :
    m_scheduler(scheduler),
    m_state(ProcessState::Stopped),
    m_exitCode(0),
    m_unexpectedExit(false)
{
    m_name = conf.getAttr("", "name");
    m_executable = conf.get("executable");
    m_rootPath = conf.get("root");
    m_dataRootPath = conf.get("data");

    m_cmdLineFixedArgs = conf.get("args", std::string());

    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &schemeConf, conf.getRangeOf("options.scheme"))
    {
        const std::string name = schemeConf.getAttr("", "name");
        IConfigSchemePtr scheme = ConfigScheme::createFromConfig(schemeConf);
        IOptionsStoragePtr storage(new OptionsStorage(scheme, true));
        m_configuration.addStorage(name, storage);
    }

    m_substituteHandlers["PID"] = &ProcessBase::substitutePID;
    m_substituteHandlers["ROOTPATH"] = boost::bind(&ProcessBase::substituteRootPath, this);
    m_substituteHandlers["DATAROOTPATH"] = boost::bind(&ProcessBase::substituteDataRootPath, this);
    m_substituteHandlers["CONFIGFILE"] = boost::bind(&ProcessBase::substituteConfigFilePath, this);
    m_substituteHandlers["LOGFILE"] = boost::bind(&ProcessBase::substituteLogFilePath, this, LogFilePathPart::Full);
    m_substituteHandlers["LOGFILENAME"] = boost::bind(&ProcessBase::substituteLogFilePath, this, LogFilePathPart::Name);
    m_substituteHandlers["LOGFILELOCATION"] = boost::bind(&ProcessBase::substituteLogFilePath, this, LogFilePathPart::Location);

    m_scheduler.add_active_user();
}

//-------------------------------------------------------------------------------------------------
ProcessBase::~ProcessBase()
{
    m_scheduler.remove_active_user();
    
    if (isRunningInternal())
    {
        stop(StopProcessHandler());
        m_childPtr->join();
    }
    // Remove old config and log
    try
    {
        if (!m_logFilePath.empty() && fs::exists(m_logFilePath))
        {
            fs::remove(m_logFilePath);
        }
        if (!m_configFilePath.empty() && fs::exists(m_configFilePath))
        {
            fs::remove(m_configFilePath);
        }
    }
    catch (const fs::filesystem_error &e)
    {
        Tools::Logger::Logger::getInstance().warning(std::string("Can't delete file ") + e.path1().string() + ": " + e.what());
    }
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
const boost::filesystem::path &ProcessBase::dataRootPath() const
{
    return m_dataRootPath;
}

//-------------------------------------------------------------------------------------------------
const ProcessConfiguration &ProcessBase::getConfiguration() const
{
    return m_configuration;
}

//-------------------------------------------------------------------------------------------------
class AsyncProcessHandler : public ex::async_handler
{
public:
    typedef boost::function2<void, int, const std::error_code &> OnExitHandler;
    typedef boost::function0<void> OnSuccessHandler;
    typedef boost::function1<void, const std::error_code &> OnErrorHandler;

    AsyncProcessHandler(const OnExitHandler &onExit,
        const OnSuccessHandler &onSuccess,
        const OnErrorHandler &onError):
        m_onExit(onExit), m_onSuccess(onSuccess), m_onError(onError) {}

    virtual ~AsyncProcessHandler() {}

    template<typename Executor>
    std::function<void(int, const std::error_code&)> on_exit_handler(Executor &exec)
    {
        auto handler = this->m_onExit;
        return [handler](int exitCode, const std::error_code &ec)
        {
            handler(exitCode, ec);
        };
    }

    template <class Executor>
    void on_setup(Executor &exec) const 
    {
        exec.creation_flags |= CREATE_NO_WINDOW;
        exec.startup_info.dwFlags |= STARTF_USESHOWWINDOW;
        exec.startup_info.wShowWindow |= SW_MINIMIZE;
    }

    template <class Executor>
    void on_error(Executor &exec, const std::error_code &ec) const
    {
        boost::asio::io_service &ios = ex::get_io_service(exec.seq);
        ios.post(boost::bind<>(m_onError, ec));
    }

    template <class Executor>
    void on_success(Executor &exec) const 
    {
        boost::asio::io_service &ios = ex::get_io_service(exec.seq);
        ios.post(m_onSuccess);
    }

private:
    OnExitHandler m_onExit;
    OnSuccessHandler m_onSuccess;
    OnErrorHandler m_onError;
};

//-------------------------------------------------------------------------------------------------
void ProcessBase::start(const StartProcessHandler &handler)
{
    UniqueLockType lock(m_access);

    if (isRunningInternal())
    {
        throw ProcessError(makeErrorCode(ProcessErrors::alreadyRunning));
    }
    
    // Remove old config and log
    try
    {
        if (!m_logFilePath.empty() && fs::exists(m_logFilePath))
        {
            fs::remove(m_logFilePath);
        }
        if (!m_configFilePath.empty() && fs::exists(m_configFilePath))
        {
            fs::remove(m_configFilePath);
        }
    }
    catch (const fs::filesystem_error &e)
    {
        Tools::Logger::Logger::getInstance().warning(std::string("Can't delete file ") + e.path1().string() + ": " + e.what());
    }

    // Set it before createConfigFile()!
    m_logFilePath = dataRootPath() / boost::filesystem::unique_path(name() + "-%%%%-%%%%-%%%%-%%%%.log");

    if (hasConfigFile())
    {
        createConfigFile();
    }

    std::string exePath = (rootPath() / executable()).string();
    if (exePath.find(' ') != std::string::npos)
    {
        exePath = "\"" + exePath + "\"";
    }

    std::vector<std::string> args;

    if (!m_cmdLineFixedArgs.empty())
    {
        args.push_back(m_cmdLineFixedArgs);
    }
    
    if (m_configuration.hasStorage(s_cmdLineSection))
    {
        IOptionsStoragePtr configPtr = m_configuration.getStorage(s_cmdLineSection);
        IConfigSchemePtr schemePtr = configPtr->getScheme();

        for (const OptionDesc &od : schemePtr->getFilteredRange<>(OptionIsRequiredPred()))
        {
            const std::string &name = od.get<0>();
            if (!configPtr->hasValue(name))
            {
                throw ProcessError(makeErrorCode(ProcessErrors::missingRequiredOption), "Required option " + name + " has no value.");
            }
        }

        BOOST_FOREACH(const Option &o, configPtr->getRange())
        {
            const std::string formattedString = configPtr->formatOption(o.name(), shared_from_this());
            args.push_back(formattedString);
        }

    }
    std::string command = exePath;
    for (const std::string &i : args)
    {
        command += " " + i;
    }

    Tools::Logger::Logger::getInstance().info("Starting process " + command);
    
    setState(ProcessState::Starting);
    m_exitCode = 0;
    m_exitErrorCode = std::error_code();
    m_unexpectedExit = false;

    m_childPtr.reset(new bp::child(bp::cmd = command,
        m_scheduler.get_io_service(),
        AsyncProcessHandler(boost::bind<>(&ProcessBase::onProcessExit, shared_from_this(), _1, _2),
            boost::bind<>(&ProcessBase::onProcessStart, shared_from_this(), handler, std::error_code()),
            boost::bind<>(&ProcessBase::onProcessStart, shared_from_this(), handler, _1))
        ));
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::onProcessStart(const StartProcessHandler &handler, const std::error_code &ec)
{
    UniqueLockType lock(m_access);

    if (ec)
    {
        setState(ProcessState::Stopped);
        m_childPtr.reset();
    }
    else
    {
        setState(ProcessState::Running);
    }
    handler(ec);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::onProcessExit(int exitCode, const std::error_code &ec)
{
    UniqueLockType lock(m_access);
    if (m_state != ProcessState::Stopping)
    {
        m_unexpectedExit = true;
    }
    m_exitCode = exitCode;
    m_exitErrorCode = ec;
    setState(ProcessState::Stopped);
    m_childPtr.reset();
    if (!m_stopHandler.empty())
    {
        m_stopHandler(ec, ExitStatus(m_exitCode, ec, m_unexpectedExit));
        m_stopHandler.clear();
    }
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
    
    IOptionsStoragePtr configPtr = m_configuration.getStorage(s_configFileSection);
    IConfigSchemePtr schemePtr = configPtr->getScheme();
    
    for (const OptionDesc &od : schemePtr->getFilteredRange<>(OptionIsRequiredPred()))
    {
        const std::string &name = od.get<0>();
        if (!configPtr->hasValue(name))
        {
            throw ProcessError(makeErrorCode(ProcessErrors::missingRequiredOption), "Required option " + name + " has no value.");
        }
    }

    const boost::filesystem::path outFilePath = dataRootPath() / boost::filesystem::unique_path(name() + "-%%%%-%%%%-%%%%-%%%%.config");

    std::ofstream outFile(outFilePath.c_str(), std::ios_base::out | std::ios_base::trunc);

    if (!outFile)
    {
        throw ProcessError(makeErrorCode(ProcessErrors::configFileWriteError), "Can't create file " + outFilePath.string());
    }

    BOOST_FOREACH(const Option &o, configPtr->getRange())
    {
        const std::string formattedString = configPtr->formatOption(o.name(), shared_from_this());
        outFile << formattedString << std::endl;
    }
    outFile.close();

    m_configFilePath = outFilePath;

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
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage),
            name() + "." + configName);
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
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage),
            name() + "." + configName);
    }

    IOptionsStorageConstPtr storagePtr = m_configuration.getStorage(configName);

    if (!storagePtr->getScheme()->hasOption(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchOption),
            name() + "." + configName + "." + optionName);
    }

    return storagePtr->getScheme()->getOptionDesc(optionName);
}

//-------------------------------------------------------------------------------------------------
OptionDescValue ProcessBase::getOptionValue(const std::string &configName, const std::string &optionName) const
{
    SharedLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage),
            name() + "." + configName);
    }

    IOptionsStorageConstPtr storagePtr = m_configuration.getStorage(configName);
    IConfigSchemePtr schemePtr = storagePtr->getScheme();

    if (!schemePtr->hasOption(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchOption),
            name() + "." + configName + "." + optionName);
    }

    const OptionDesc desc = storagePtr->getScheme()->getOptionDesc(optionName);
    const OptionValueType value = storagePtr->hasValue(optionName) ? storagePtr->getValue(optionName) :
        (schemePtr->hasDefaultValue(optionName) ? schemePtr->getDefaultValue(optionName) : OptionValueType());
    const std::string presentation = storagePtr->hasValue(optionName) ? storagePtr->formatOption(optionName, shared_from_this()) : "";
    return OptionDescValue(desc, value, presentation);
}

//-------------------------------------------------------------------------------------------------
ProcessState ProcessBase::getState() const
{
    return m_state;
}

//-------------------------------------------------------------------------------------------------
bool ProcessBase::hasSubstitute(const std::string &value) const
{
    return m_substituteHandlers.find(value) != m_substituteHandlers.end();
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substituteValue(const std::string &value) const
{
    SubstituteHandlers::const_iterator it = m_substituteHandlers.find(value);
    if (it == m_substituteHandlers.end())
    {
        throw ProcessError(makeErrorCode(ProcessErrors::substitutionNotFound), "Process " + name() + " has no substitution for " + value);
    }
    return (it->second)();
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substitutePID()
{
    return boost::lexical_cast<std::string>(GetCurrentProcessId());
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substituteRootPath() const
{
    return rootPath().string();
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substituteDataRootPath() const
{
    return dataRootPath().string();
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substituteConfigFilePath() const
{
    return m_configFilePath.string();
}

//-------------------------------------------------------------------------------------------------
std::string ProcessBase::substituteLogFilePath(LogFilePathPart part) const
{
    switch (part)
    {
    case LogFilePathPart::Full:
        return m_logFilePath.string();
    case LogFilePathPart::Name:
        return m_logFilePath.filename().string();
    case LogFilePathPart::Location:
        return m_logFilePath.parent_path().string();
    }
    BOOST_ASSERT(false);
    return m_logFilePath.string();
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::setState(ProcessState newState)
{
    m_state = newState;
}

//-------------------------------------------------------------------------------------------------
ExitStatus ProcessBase::getExitStatus() const
{
    SharedLockType lock(m_access);
    return ExitStatus(m_exitCode, m_exitErrorCode, m_unexpectedExit);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::stop(const StopProcessHandler &handler)
{
    UniqueLockType lock(m_access);

    if (!isRunningInternal() || getState() != ProcessState::Running)
    {
        throw ProcessError(makeErrorCode(ProcessErrors::processNotRunning));
    }

    BOOST_ASSERT(m_stopHandler.empty());

    m_stopHandler = handler;
    setState(ProcessState::Stopping);
    std::error_code ec;
    m_childPtr->terminate(ec);
}

//-------------------------------------------------------------------------------------------------
OptionDescValue ProcessBase::setOptionValue(const std::string &configName,
    const std::string &optionName,
    const OptionValueContainer &optionValue)
{
    UniqueLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage),
            name() + "." + configName);
    }

    if (getState() != ProcessState::Stopped &&
        (configName == s_configFileSection || configName == s_cmdLineSection))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::cantEditConfigOfRunningProcess),
            name() + "." + configName);
    }

    IOptionsStoragePtr storagePtr = m_configuration.getStorage(configName);
    IConfigSchemePtr schemePtr = storagePtr->getScheme();

    if (!schemePtr->hasOption(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchOption),
            name() + "." + configName + "." + optionName);
    }

    if (schemePtr->isSystem(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::systemOptionEditForbidden),
            name() + "." + configName + "." + optionName);
    }

    storagePtr->setValue(optionName, optionValue);

    const OptionDesc desc = storagePtr->getScheme()->getOptionDesc(optionName);
    const OptionValueType value = storagePtr->hasValue(optionName) ? storagePtr->getValue(optionName) :
        (schemePtr->hasDefaultValue(optionName) ? schemePtr->getDefaultValue(optionName) : OptionValueType());
    const std::string presentation = storagePtr->hasValue(optionName) ? storagePtr->formatOption(optionName, shared_from_this()) : "";

    return OptionDescValue(desc, value, presentation);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::applyConfig(const ProcessConfiguration &presetConf)
{
    UniqueLockType lock(m_access);
    applyConfigImpl(presetConf, false);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::applyUserConfig(const ProcessConfiguration &presetConf)
{
    UniqueLockType lock(m_access);
    applyConfigImpl(presetConf, true);
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::applyConfigImpl(const ProcessConfiguration &presetConf, bool saveCurrentOptions)
{
    if (isRunningInternal() || getState() != ProcessState::Stopped)
    {
        throw ProcessError(makeErrorCode(ProcessErrors::alreadyRunning));
    }

    std::map<std::string, IOptionsStoragePtr> newStorages;

    for (IOptionsStoragePtr currentStorage : m_configuration.getRange())
    {
        const std::string storageName = currentStorage->getScheme()->name();
        IConfigSchemePtr schemePtr = currentStorage->getScheme();
        IOptionsStoragePtr newStorage(new OptionsStorage(schemePtr, true));

        // Copy current options
        if (saveCurrentOptions)
        {
            for (const Option &opt : currentStorage->getRange())
            {
                newStorage->setValue(opt.name(), opt.value());
            }
        }
        
        // Copy preset options
        if (presetConf.hasStorage(storageName))
        {
            IOptionsStorageConstPtr presetsStorage = presetConf.getStorage(storageName);
            for (const Option &opt : presetsStorage->getRange())
            {
                newStorage->setValue(opt.name(), opt.value());
            }
        }

        newStorages[storageName] = newStorage;
    }

    std::list<std::string> storages;
    m_configuration.getStorages(storages);
    for (const std::string &storageName : storages)
    {
        m_configuration.removeStorage(storageName);
    }

    for (std::map<std::string, IOptionsStoragePtr>::value_type &i : newStorages)
    {
        m_configuration.addStorage(i.first, i.second);
    }
}

//-------------------------------------------------------------------------------------------------
void ProcessBase::getLog(LogLineHandler &acc)
{
    if (m_logFilePath.empty())
    {
        // TODO: throw?
        return;
    }

    fs::ifstream logFile(m_logFilePath, std::ios_base::in);
    std::string line;
    while (std::getline(logFile, line))
    {
        acc(line);
    }
}

//-------------------------------------------------------------------------------------------------
OptionDescValue ProcessBase::removeOptionValue(const std::string &configName, const std::string &optionName)
{
    UniqueLockType lock(m_access);
    if (!m_configuration.hasStorage(configName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage),
            name() + "." + configName);
    }

    if (getState() != ProcessState::Stopped &&
        (configName == s_configFileSection || configName == s_cmdLineSection))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::cantEditConfigOfRunningProcess),
            name() + "." + configName);
    }

    IOptionsStoragePtr storagePtr = m_configuration.getStorage(configName);
    IConfigSchemePtr schemePtr = storagePtr->getScheme();

    if (!schemePtr->hasOption(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchOption),
            name() + "." + configName + "." + optionName);
    }

    if (schemePtr->isSystem(optionName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::systemOptionEditForbidden),
            name() + "." + configName + "." + optionName);
    }

    if (storagePtr->hasValue(optionName))
    {
        storagePtr->removeValue(optionName);
    }

    const OptionDesc desc = storagePtr->getScheme()->getOptionDesc(optionName);
    const OptionValueType value = storagePtr->hasValue(optionName) ? storagePtr->getValue(optionName) :
        (schemePtr->hasDefaultValue(optionName) ? schemePtr->getDefaultValue(optionName) : OptionValueType());
    const std::string presentation = storagePtr->hasValue(optionName) ? storagePtr->formatOption(optionName, shared_from_this()) : "";

    return OptionDescValue(desc, value, presentation);
}
