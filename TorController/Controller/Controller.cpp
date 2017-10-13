#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>
#include <boost/process/exception.hpp>

#include "Controller/Controller.h"

namespace bp = boost::process;

//-------------------------------------------------------------------------------------------------
Controller::Controller(const Tools::Configuration::ConfigurationView &config) :
    m_config(config), m_logger(Tools::Logger::Logger::getInstance())
{
	// TODO: read config
	m_scheduler.set_num_threads(4);
	m_scheduler.add_active_user();

    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &processConf, config.getRangeOf("processes.process"))
    {
        ProcessBasePtr processPtr(new ProcessBase(processConf, m_scheduler));
        m_processes[processPtr->name()] = processPtr;
    }

}

//-------------------------------------------------------------------------------------------------
Controller::~Controller()
{
	m_scheduler.remove_active_user();
	m_scheduler.shutdown();
	m_scheduler.join();
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
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);

        Processes::const_iterator i = m_processes.find(name);
        if (i == m_processes.end())
        {
            throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
        }

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
    boost::shared_lock<boost::upgrade_mutex> lock(m_access);

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
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);
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
    boost::shared_lock<boost::upgrade_mutex> lock(m_access);

    Processes::const_iterator i = m_processes.find(processName);
    if (i == m_processes.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound));
    }

    IProcessPtr processPtr = i->second;
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
    const GetProcessOptionResult::Handler &handler)
{
    GetProcessOptionResult result;
    boost::shared_lock<boost::upgrade_mutex> lock(m_access);

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
                                  const GetProcessOptionResult::Handler &handler)
{
    safeActionCall<GetProcessOptionResult>(
        boost::bind(&Controller::getProcessOptionImpl, this, processName, configName, optionName, handler), 
        handler);
}

//-------------------------------------------------------------------------------------------------
void Controller::startProcessHandler(const StartProcessResult::Handler &handler, const ErrorCode &ec)
{
    StartProcessResult result;

    scheduleActionHandler<>(handler, result);
}