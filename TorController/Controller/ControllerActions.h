#pragma once

#include <map>
#include <string>

#include <json/value.h>

#include <boost/assert.hpp>
#include <boost/function/function1.hpp>

#include "Options/IConfigScheme.h"
#include "Error.h"
#include "Process/IProcess.h"

//-------------------------------------------------------------------------
class ActionResult
{
public:
    ActionResult();

    explicit ActionResult(const ErrorCode &ec);

    virtual ~ActionResult();

    virtual Json::Value toJson() const = 0;

    void setError(const ErrorCode &ec);

    const ErrorCode &getError() const;

private:
    ErrorCode m_ec;
};

//-------------------------------------------------------------------------
class ControllerInfoResult : public ActionResult
{
public:
    ControllerInfoResult();

    explicit ControllerInfoResult(const ErrorCode &ec);

    virtual ~ControllerInfoResult();

    Json::Value toJson() const override;

    std::int64_t m_pid;

    typedef boost::function1<void, ControllerInfoResult> Handler;
};

//-------------------------------------------------------------------------
class StartProcessResult : public ActionResult
{
public:
    StartProcessResult();

    explicit StartProcessResult(const ErrorCode &ec);

    virtual ~StartProcessResult();

    Json::Value toJson() const override;

    typedef boost::function1<void, StartProcessResult> Handler;
};

//-------------------------------------------------------------------------
class StopProcessResult : public ActionResult
{
public:
    StopProcessResult();

    explicit StopProcessResult(const ErrorCode &ec);

    virtual ~StopProcessResult();

    Json::Value toJson() const override;

    typedef boost::function1<void, StopProcessResult> Handler;

    ExitStatus m_exitStatus;
};

//-------------------------------------------------------------------------
class GetProcessesResult : public ActionResult
{
public:
    GetProcessesResult();

    explicit GetProcessesResult(const ErrorCode &ec);

    virtual ~GetProcessesResult();

    Json::Value toJson() const override;

    std::list<std::string> m_processes;

    typedef boost::function1<void, GetProcessesResult> Handler;
};

//-------------------------------------------------------------------------
class GetProcessInfoResult : public ActionResult
{
public:
    GetProcessInfoResult();

    explicit GetProcessInfoResult(const ErrorCode &ec);

    virtual ~GetProcessInfoResult();

    Json::Value toJson() const override;

    typedef boost::function1<void, GetProcessInfoResult> Handler;

    std::string m_name;
    ProcessState m_state;
    std::list<std::string> m_configs;
};

//-------------------------------------------------------------------------
class GetProcessConfigsResult : public ActionResult
{
public:
    GetProcessConfigsResult();

    explicit GetProcessConfigsResult(const ErrorCode &ec);

    virtual ~GetProcessConfigsResult();

    Json::Value toJson() const override;

    std::list<std::string> m_configs;
    
    typedef boost::function1<void, GetProcessConfigsResult> Handler;
};

//-------------------------------------------------------------------------
class GetProcessConfigResult : public ActionResult
{
public:
    GetProcessConfigResult();

    explicit GetProcessConfigResult(const ErrorCode &ec);

    virtual ~GetProcessConfigResult();

    Json::Value toJson() const override;

    std::string m_name;
    std::list<std::string> m_options;

    typedef boost::function1<void, GetProcessConfigResult> Handler;
};

//-------------------------------------------------------------------------
class ProcessOptionResult : public ActionResult
{
public:
    ProcessOptionResult();

    explicit ProcessOptionResult(const ErrorCode &ec);

    virtual ~ProcessOptionResult();

    Json::Value toJson() const override;

    OptionDescValue m_option;

    typedef boost::function1<void, ProcessOptionResult> Handler;
};


