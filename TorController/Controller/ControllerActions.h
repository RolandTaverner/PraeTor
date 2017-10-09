#pragma once

#include <map>
#include <string>

#include <json/value.h>

#include <boost/assert.hpp>
#include <boost/function/function1.hpp>

#include "Options/IConfigScheme.h"

//-------------------------------------------------------------------------
class ActionResult
{
public:
    ActionResult();

    explicit ActionResult(const std::error_condition &ec);

    virtual ~ActionResult();

    virtual Json::Value toJson() const = 0;

    void setError(const std::error_condition &ec);

    const std::error_condition &getError() const;

private:
    std::error_condition m_ec;
};

//-------------------------------------------------------------------------
class StartProcessResult : public ActionResult
{
public:
    StartProcessResult();

    explicit StartProcessResult(const std::error_condition &ec);

    virtual ~StartProcessResult();

    Json::Value toJson() const override;
};
typedef boost::function1<void, StartProcessResult> StartProcessHandler;

//-------------------------------------------------------------------------
class ControllerInfoResult : public ActionResult
{
public:
    ControllerInfoResult();

    explicit ControllerInfoResult(const std::error_condition &ec);

    virtual ~ControllerInfoResult();

    Json::Value toJson() const override;

    std::int64_t m_pid;
};
typedef boost::function1<void, ControllerInfoResult> ControllerInfoHandler;

//-------------------------------------------------------------------------
class StopProcessResult : public ActionResult
{
public:
    StopProcessResult();

    explicit StopProcessResult(const std::error_condition &ec);

    virtual ~StopProcessResult();

    Json::Value toJson() const override;
};
typedef boost::function1<void, StopProcessResult> StopProcessHandler;

//-------------------------------------------------------------------------
class GetProcessesResult : public ActionResult
{
public:
    GetProcessesResult();

    explicit GetProcessesResult(const std::error_condition &ec);

    virtual ~GetProcessesResult();

    Json::Value toJson() const override;

    std::list<std::string> m_processes;
};
typedef boost::function1<void, GetProcessesResult> GetProcessesHandler;

//-------------------------------------------------------------------------
class GetProcessInfoResult : public ActionResult
{
public:
    GetProcessInfoResult();

    explicit GetProcessInfoResult(const std::error_condition &ec);

    virtual ~GetProcessInfoResult();

    Json::Value toJson() const override;
};
typedef boost::function1<void, GetProcessInfoResult> GetProcessInfoHandler;

//-------------------------------------------------------------------------
class GetProcessConfigsResult : public ActionResult
{
public:
    GetProcessConfigsResult();

    explicit GetProcessConfigsResult(const std::error_condition &ec);

    virtual ~GetProcessConfigsResult();

    Json::Value toJson() const override;

    std::list<std::string> m_configs;
};
typedef boost::function1<void, GetProcessConfigsResult> GetProcessConfigsHandler;

//-------------------------------------------------------------------------
class GetProcessConfigResult : public ActionResult
{
public:
    GetProcessConfigResult();

    explicit GetProcessConfigResult(const std::error_condition &ec);

    virtual ~GetProcessConfigResult();

    Json::Value toJson() const override;

    std::list<std::string> m_options;
};
typedef boost::function1<void, GetProcessConfigResult> GetProcessConfigHandler;

//-------------------------------------------------------------------------
class GetProcessOptionResult : public ActionResult
{
public:
    GetProcessOptionResult();

    explicit GetProcessOptionResult(const std::error_condition &ec);

    virtual ~GetProcessOptionResult();

    Json::Value toJson() const override;

    OptionDescValue m_option;
};
typedef boost::function1<void, GetProcessOptionResult> GetProcessOptionHandler;


