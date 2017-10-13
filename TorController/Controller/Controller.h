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
#include "Tools/Logger/Logger.h"

#include "Controller/ControllerActions.h"
#include "Controller/ControllerErrors.h"
#include "Process/ProcessBase.h"

class Controller : private boost::noncopyable
{
public:
    explicit Controller(const Tools::Configuration::ConfigurationView &config);
    virtual ~Controller();

    void getControllerInfo(const ControllerInfoResult::Handler &handler);
    void getProcesses(const GetProcessesResult::Handler &handler);
    void getProcessInfo(const std::string &name, const GetProcessInfoResult::Handler &handler);
    void getProcessConfigs(const std::string &name, const GetProcessConfigsResult::Handler &handler);
    void getProcessConfig(const std::string &processName, const std::string &configName, const GetProcessConfigResult::Handler &handler);
    void getProcessOption(const std::string &processName,
                          const std::string &configName,
                          const std::string &optionName,
                          const GetProcessOptionResult::Handler &handler);

    void startProcess(const std::string &name, const StartProcessResult::Handler &handler);
    void stopProcess(const std::string &name, const StopProcessResult::Handler &handler);

    const Tools::Configuration::ConfigurationView &getConf() const;

private:
    void getControllerInfoImpl(const ControllerInfoResult::Handler &handler);
    void getProcessesImpl(const GetProcessesResult::Handler &handler);
    void getProcessInfoImpl(const std::string &name, const GetProcessInfoResult::Handler &handler);
    void getProcessConfigsImpl(const std::string &name, const GetProcessConfigsResult::Handler &handler);
    void getProcessConfigImpl(const std::string &processName, const std::string &configName, const GetProcessConfigResult::Handler &handler);
    void getProcessOptionImpl(const std::string &processName,
        const std::string &configName,
        const std::string &optionName,
        const GetProcessOptionResult::Handler &handler);

    void startProcessImpl(const std::string &name, const StartProcessResult::Handler &handler);
    void stopProcessImpl(const std::string &name, const StopProcessResult::Handler &handler);

private:
	pion::single_service_scheduler m_scheduler;

	template<typename ActionHandler, typename ActionResult>
	void scheduleActionHandler(const ActionHandler &handler, const ActionResult &result)
	{
		m_scheduler.post(boost::bind(handler, result));
	}

    void startProcessHandler(const StartProcessResult::Handler &handler, const ErrorCode &ec);

    template<typename ActionResultType>
    void safeActionCall(const boost::function0<void> &call, const typename ActionResultType::Handler &handler)
    {
        ErrorCode errorCode;

        try
        {
            call();
            return;
        }
        catch (const boost::system::system_error &e)
        {
            errorCode = ErrorCode(e.code(), e.what());
        }
        catch (const std::system_error &e)
        {
            errorCode = ErrorCode(e.code(), e.what());
        }
        catch (const std::exception &e)
        {
            errorCode = ErrorCode(makeErrorCode(ControllerErrors::unknownError), e.what());
        }

        if (errorCode)
        {
            m_logger.error() << "Error. Category: " << errorCode.category()
                << ". Message: " << errorCode.message();
            scheduleActionHandler<>(handler, ActionResultType(errorCode));
        }
    }
	
        
    Tools::Configuration::ConfigurationView m_config;
    typedef std::map<std::string, ProcessBasePtr> Processes;
    Processes m_processes;

	typedef boost::shared_mutex MutexType;
	typedef boost::shared_lock_guard<MutexType> SharedLockType;
	typedef boost::lock_guard<MutexType> UniqueLockType;

    Tools::Logger::Logger &m_logger;

	mutable MutexType m_access;
};

typedef boost::shared_ptr<Controller> ControllerPtr;