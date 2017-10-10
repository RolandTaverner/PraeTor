#pragma once

#include <map>
#include <string>
#include <ostream>

#include <json/value.h>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/function/function1.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <pion/scheduler.hpp>

#include "Tools/Configuration/ConfigurationView.h"

#include "Controller/ControllerActions.h"
#include "Controller/ControllerErrors.h"
#include "Process/ProcessBase.h"

class Controller : private boost::noncopyable
{
public:
    explicit Controller(const Tools::Configuration::ConfigurationView &config);
    virtual ~Controller();

    void getControllerInfo(const ControllerInfoHandler &handler);
    void getProcesses(const GetProcessesHandler &handler);
    void getProcessInfo(const std::string &name, const GetProcessInfoHandler &handler);
    void getProcessConfigs(const std::string &name, const GetProcessConfigsHandler &handler);
    void getProcessConfig(const std::string &processName, const std::string &configName, const GetProcessConfigHandler &handler);
    void getProcessOption(const std::string &processName,
                          const std::string &configName,
                          const std::string &optionName,
                          const GetProcessOptionHandler &handler);

    void startProcess(const std::string &name, const StartProcessHandler &handler);
    void stopProcess(const std::string &name, const StopProcessHandler &handler);

    const Tools::Configuration::ConfigurationView &getConf() const;

private:
	pion::single_service_scheduler m_scheduler;

	template<typename ActionHandler, typename ActionResult>
	void scheduleActionHandler(const ActionHandler &handler, const ActionResult &result)
	{
		m_scheduler.post(boost::bind(handler, result));
	}

    void startProcessHandler(const StartProcessHandler &handler, const std::error_condition &ec);

	Tools::Configuration::ConfigurationView m_config;
    typedef std::map<std::string, ProcessBasePtr> Processes;
    Processes m_processes;

	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock_guard<MutexType> SharedLockType;
	typedef boost::lock_guard<MutexType> UniqueLockType;

	mutable MutexType m_access;
};

typedef boost::shared_ptr<Controller> ControllerPtr;