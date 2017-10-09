#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>
#include <boost/process/exception.hpp>

#include "Controller/Controller.h"

namespace bp = boost::process;

//-------------------------------------------------------------------------------------------------
Controller::Controller(const Tools::Configuration::ConfigurationView &config) :
    m_config(config)
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
void Controller::startProcess(const std::string &name, const StartProcessHandler &handler)
{
	UniqueLockType lock(m_access);

	Processes::const_iterator i = m_processes.find(name);
	if (i == m_processes.end())
	{
		scheduleActionHandler<>(handler, StartProcessResult(makeErrorCondition(ControllerErrors::processNotFound)));
		return;
	}

	try
	{
		//i->second->start();
	}
	catch (const boost::system::system_error &e)
	{
		scheduleActionHandler<>(handler, StartProcessResult(makeErrorCondition(ControllerErrors::startProcessError)));
	}
	catch (const bp::process_error &e)
	{
		scheduleActionHandler<>(handler, StartProcessResult(makeErrorCondition(ControllerErrors::startProcessError)));
	}
	catch (const std::exception &e)
	{
		scheduleActionHandler<>(handler, StartProcessResult(makeErrorCondition(ControllerErrors::startProcessError)));
	}
}

//-------------------------------------------------------------------------------------------------
void Controller::stopProcess(const std::string &name, const StopProcessHandler &handler)
{

}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessInfo(const std::string &name, const GetProcessInfoHandler &handler)
{
    GetProcessInfoResult result;
    {
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);

        Processes::const_iterator i = m_processes.find(name);
        if (i == m_processes.end())
        {
            //result.ec = 
        }
        else
        {

        }
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfigs(const std::string &name, const GetProcessConfigsHandler &handler)
{
    GetProcessConfigsResult result;
    {
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);

        Processes::const_iterator i = m_processes.find(name);
        if (i == m_processes.end())
        {
            result.setError(makeErrorCondition(ControllerErrors::processNotFound));
        }
        else
        {
            IProcessPtr processPtr = i->second;
            processPtr->getConfigurations(result.m_configs);
        }
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcesses(const GetProcessesHandler &handler)
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
void Controller::getControllerInfo(const ControllerInfoHandler &handler)
{
    ControllerInfoResult result;
    result.m_pid = GetCurrentProcessId();

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessConfig(const std::string &processName,
                                  const std::string &configName,
                                  const GetProcessConfigHandler &handler)
{
    GetProcessConfigResult result;
    {
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);

        Processes::const_iterator i = m_processes.find(processName);
        if (i == m_processes.end())
        {
            result.setError(makeErrorCondition(ControllerErrors::processNotFound));
        }
        else
        {
            IProcessPtr processPtr = i->second;
            processPtr->getConfigurationOptions(configName, result.m_options);
        }
    }

    scheduleActionHandler<>(handler, result);
}

//-------------------------------------------------------------------------------------------------
void Controller::getProcessOption(const std::string &processName,
                                  const std::string &configName,
                                  const std::string &optionName,
                                  const GetProcessOptionHandler &handler)
{
    GetProcessOptionResult result;
    {
        boost::shared_lock<boost::upgrade_mutex> lock(m_access);

        Processes::const_iterator i = m_processes.find(processName);
        if (i == m_processes.end())
        {
            result.setError(makeErrorCondition(ControllerErrors::processNotFound));
        }
        else
        {
            IProcessPtr processPtr = i->second;
            result.m_option = processPtr->getOptionValue(configName, optionName);
        }
    }

    scheduleActionHandler<>(handler, result);
}
