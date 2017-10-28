#include <boost/bind.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>
#include <boost/process/exception.hpp>

#include "Tools/Configuration/XmlParser.h"

#include "Controller/Controller.h"

namespace bp = boost::process;
namespace fs = boost::filesystem;

static const char s_presetsFileName[] = "userpresets.xml";
static const char s_userPresetsGroupName[] = "user";

//-------------------------------------------------------------------------------------------------
Controller::Controller(const Tools::Configuration::ConfigurationView &config) :
    m_config(config), m_logger(Tools::Logger::Logger::getInstance())
{
    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &processConf, getConf().getRangeOf("processes.process"))
    {
        ProcessBasePtr processPtr(new ProcessBase(processConf, m_scheduler));
        m_processes[processPtr->name()] = processPtr;
    }

    m_installRoot = getConf().get("installroot");
    m_dataRoot = getConf().get("dataroot");

    if (getConf().exists("presets"))
    {
        m_presets.load(getConf().branch("presets"), getConf().branch("processes"));
    }

    try
    {
        loadUserPresets(getConf().branch("processes"));
    }
    catch (const std::exception &e)
    {
        m_userPresets = Presets::createTemplate(s_userPresetsGroupName, getConf().branch("processes"));
    }

    if (!m_userPresets.getRange().empty())
    {
        const PresetGroup &pg = m_userPresets.getPresets(s_userPresetsGroupName);
        const PresetGroupConfig &presetGroup = pg.second;
        
        for (Processes::iterator it = m_processes.begin(); it != m_processes.end(); ++it)
        {
            const std::string processName = it->first;
            IProcessPtr processPtr = it->second;

            PresetGroupConfig::const_iterator itProcConf = presetGroup.find(processName);
            if (itProcConf != presetGroup.end())
            {
                const ProcessConfiguration &processConfig = itProcConf->second;
                processPtr->applyUserConfig(processConfig);
            }
        }

    }
    // TODO: read config
    m_scheduler.set_num_threads(4);
    m_scheduler.add_active_user();
}

//-------------------------------------------------------------------------------------------------
Controller::~Controller()
{
    m_scheduler.remove_active_user();
    m_scheduler.shutdown();
    m_scheduler.join();
}

//-------------------------------------------------------------------------------------------------
void Controller::loadUserPresets(const Tools::Configuration::ConfigurationView &processesConf)
{
    const fs::path presetsFilePath = fs::path(m_dataRoot) / s_presetsFileName;

    fs::ifstream presetFile(presetsFilePath, std::ios_base::in);
    if (!presetFile)
    {
        throw std::runtime_error("User presets file " + presetsFilePath.string() + " not found");
    }

    Tools::Configuration::ConfigurationView userPresetsConf = Tools::Configuration::Parsers::Xml::readXml(presetFile);
    m_userPresets.load(userPresetsConf.branch("presets"), processesConf, true);
}

//-------------------------------------------------------------------------------------------------
void Controller::saveUserPresets()
{
    const fs::path presetsFilePath = fs::path(m_dataRoot) / s_presetsFileName;
    fs::ofstream presetFile(presetsFilePath, std::ios_base::out | std::ios_base::trunc);
    if (!presetFile)
    {
        throw std::runtime_error("Can't write user presets file " + presetsFilePath.string());
    }

    Tools::Configuration::Parsers::Xml::writeXml(m_userPresets.toConfiguration(), presetFile);
}

//-------------------------------------------------------------------------------------------------
const Tools::Configuration::ConfigurationView &Controller::getConf() const
{
    return m_config;
}

//-------------------------------------------------------------------------------------------------
void Controller::startProcessImpl(const std::string &name, const StartProcessResult::Handler &handler)
{
    UniqueLockType lock(m_access);

    Processes::const_iterator i = m_processes.find(name);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }

    i->second->start(boost::bind(&Controller::startProcessHandler, this, handler, _1));
}

//-------------------------------------------------------------------------------------------------
void Controller::startProcess(const std::string &name, const StartProcessResult::Handler &handler)
{
    safeActionCall<StartProcessResult>(boost::bind(&Controller::startProcessImpl, this, name, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::stopProcessImpl(const std::string &name, const StopProcessResult::Handler &handler)
{
    UniqueLockType lock(m_access);
    Processes::const_iterator i = m_processes.find(name);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }
    
    i->second->stop(boost::bind(&Controller::stopProcessHandler, this, handler, _1, _2));
}

//-------------------------------------------------------------------------------------------------
void Controller::stopProcess(const std::string &name, const StopProcessResult::Handler &handler)
{
    safeActionCall<StopProcessResult>(boost::bind(&Controller::stopProcessImpl, this, name, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessInfoImpl(const std::string &name, const GetProcessInfoResult::Handler &handler)
{
    GetProcessInfoResult result;
    {
        SharedLockType lock(m_access);

        Processes::const_iterator i = m_processes.find(name);
        if (i == m_processes.end())
        {
            throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
        }

        result.m_name = i->first;
        result.m_state = i->second->getState();
        i->second->getConfigurations(result.m_configs);
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessInfo(const std::string &name, const GetProcessInfoResult::Handler &handler)
{
    safeActionCall<GetProcessInfoResult>(boost::bind(&Controller::getProcessInfoImpl, this, name, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfigsImpl(const std::string &name, const GetProcessConfigsResult::Handler &handler)
{
    SharedLockType lock(m_access);

    Processes::const_iterator i = m_processes.find(name);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }

    GetProcessConfigsResult result;
    IProcessPtr processPtr = i->second;
    processPtr->getConfigurations(result.m_configs);
    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfigs(const std::string &name, const GetProcessConfigsResult::Handler &handler)
{
    safeActionCall<GetProcessConfigsResult>(boost::bind(&Controller::getProcessConfigsImpl, this, name, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessesImpl(const GetProcessesResult::Handler &handler)
{
    GetProcessesResult result;
    {
        SharedLockType lock(m_access);
        BOOST_FOREACH(const Processes::value_type &v, m_processes)
        {
            result.m_processes.push_back(v.first);
        }
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcesses(const GetProcessesResult::Handler &handler)
{
    safeActionCall<GetProcessesResult>(boost::bind(&Controller::getProcessesImpl, this, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getControllerInfoImpl(const ControllerInfoResult::Handler &handler)
{
    ControllerInfoResult result;
    result.m_pid = GetCurrentProcessId();

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getControllerInfo(const ControllerInfoResult::Handler &handler)
{
    safeActionCall<ControllerInfoResult>(boost::bind(&Controller::getControllerInfoImpl, this, handler), handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfigImpl(const std::string &processName,
    const std::string &configName,
    const GetProcessConfigResult::Handler &handler)
{
    GetProcessConfigResult result;
    SharedLockType lock(m_access);

    Processes::const_iterator i = m_processes.find(processName);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }

    IProcessPtr processPtr = i->second;
    result.m_name = configName;
    processPtr->getConfigurationOptions(configName, result.m_options);

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfig(const std::string &processName,
                                  const std::string &configName,
                                  const GetProcessConfigResult::Handler &handler)
{
    safeActionCall<GetProcessConfigResult>(
        boost::bind(&Controller::getProcessConfigImpl, this, processName, configName, handler), 
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessOptionImpl(const std::string &processName,
    const std::string &configName,
    const std::string &optionName,
    const ProcessOptionResult::Handler &handler)
{
    ProcessOptionResult result;
    SharedLockType lock(m_access);

    Processes::const_iterator i = m_processes.find(processName);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }
    IProcessPtr processPtr = i->second;
    result.m_option = processPtr->getOptionValue(configName, optionName);

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessOption(const std::string &processName,
                                  const std::string &configName,
                                  const std::string &optionName,
                                  const ProcessOptionResult::Handler &handler)
{
    safeActionCall<ProcessOptionResult>(
        boost::bind(&Controller::getProcessOptionImpl, this, processName, configName, optionName, handler), 
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::startProcessHandler(const StartProcessResult::Handler &handler, const ErrorCode &ec)
{
    StartProcessResult result(ec);

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::stopProcessHandler(const StopProcessResult::Handler &handler, const ErrorCode &ec, const ExitStatus &es)
{
    StopProcessResult result(ec);
    result.m_exitStatus = es;

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::setProcessOptionImpl(const std::string &processName,
    const std::string &configName,
    const std::string &optionName,
    const OptionValueContainer &optionValue,
    const ProcessOptionResult::Handler &handler)
{
    ProcessOptionResult result;
    UniqueLockType lock(m_access);

    Processes::const_iterator i = m_processes.find(processName);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }
    IProcessPtr processPtr = i->second;
    result.m_option = processPtr->setOptionValue(configName, optionName, optionValue);

    // Save user-supplied option value
    try
    {
        m_userPresets.setOption(s_userPresetsGroupName,
            processName,
            configName,
            Option(optionName, optionValue));

        saveUserPresets();
    }
    catch (const std::exception &e)
    {
        // TODO: log this
    }
    
    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::setProcessOption(const std::string &processName,
    const std::string &configName,
    const std::string &optionName,
    const OptionValueContainer &optionValue,
    const ProcessOptionResult::Handler &handler)
{
    safeActionCall<ProcessOptionResult>(
        boost::bind(&Controller::setProcessOptionImpl, this, processName, configName, optionName, optionValue, handler),
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getPresetGroupsImpl(const PresetGroupsResult::Handler &handler)
{
    PresetGroupsResult result;
    SharedLockType lock(m_access);

    for (const PresetGroup &g : m_presets.getRange())
    {
        result.m_presetGroups.push_back(g.first);
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getPresetGroups(const PresetGroupsResult::Handler &handler)
{
    safeActionCall<PresetGroupsResult>(
        boost::bind(&Controller::getPresetGroupsImpl, this, handler),
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::applyPresetGroupImpl(const std::string &name, const ApplyPresetGroupResult::Handler &handler)
{
    ApplyPresetGroupResult result;
    UniqueLockType lock(m_access);
    const PresetGroup &pg = m_presets.getPresets(name);
    for (Processes::const_iterator it = m_processes.begin(); it != m_processes.end(); ++it)
    {
        if (it->second->getState() != ProcessState::Stopped)
        {
            throw ControllerError(makeErrorCode(ControllerErrors::processIsRunning), "Cant apply presets: process " + it->first + " is not stopped");
        }
    }

    const PresetGroupConfig &presetGroup = pg.second;

    for (Processes::iterator it = m_processes.begin(); it != m_processes.end(); ++it)
    {
        const std::string processName = it->first;
        IProcessPtr processPtr = it->second;

        PresetGroupConfig::const_iterator itProcConf = presetGroup.find(processName);
        if (itProcConf != presetGroup.end())
        {
            const ProcessConfiguration &processConfig = itProcConf->second;
            processPtr->applyConfig(processConfig);
        }
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::applyPresetGroup(const std::string &name, const ApplyPresetGroupResult::Handler &handler)
{
    safeActionCall<ApplyPresetGroupResult>(
        boost::bind(&Controller::applyPresetGroupImpl, this, name, handler),
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::getPresetsImpl(const std::string &name, const PresetsResult::Handler &handler)
{
    PresetsResult result;
    SharedLockType lock(m_access);

    const PresetGroup &pg = m_presets.getPresets(name);
    for (PresetGroupConfig::const_iterator i = pg.second.begin(); i != pg.second.end(); ++i)
    {
        const std::string &processName = i->first;
        const ProcessConfiguration &processConf = i->second;
        PresetsResult::Options processOptions;
        std::list<std::string> storages;
        processConf.getStorages(storages);
        
        PresetsResult::SchemeOptions schemeOptions;

        for (const std::string &storageName : storages)
        {
            IOptionsStorageConstPtr storagePtr = processConf.getStorage(storageName);
            PresetsResult::Options opts;
            for (const Option &o : storagePtr->getRange())
            {
                opts.push_back(o);
            }
            schemeOptions[storageName] = opts;
        }

        result.m_processOptions[processName] = schemeOptions;
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getPresets(const std::string &name, const PresetsResult::Handler &handler)
{
    safeActionCall<PresetsResult>(
        boost::bind(&Controller::getPresetsImpl, this, name, handler),
        handler);
}
