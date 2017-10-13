#include <boost/variant/static_visitor.hpp>

#include "Controller/ControllerActions.h"

//==============================================================================================================================================
ActionResult::ActionResult() {}

ActionResult::ActionResult(const ErrorCode &ec) :
    m_ec(ec)
{
}

//==============================================================================================================================================
ActionResult::~ActionResult()
{
}

//==============================================================================================================================================
void ActionResult::setError(const ErrorCode &ec)
{
    m_ec = ec;
}

//==============================================================================================================================================
const ErrorCode &ActionResult::getError() const
{
    return m_ec;
}

//==============================================================================================================================================
ControllerInfoResult::ControllerInfoResult() : m_pid(0) {}

ControllerInfoResult::ControllerInfoResult(const ErrorCode &ec) :
    ActionResult(ec), m_pid(0)
{
}

ControllerInfoResult::~ControllerInfoResult() {}

Json::Value ControllerInfoResult::toJson() const
{
    Json::Value root;
    root["PID"] = m_pid;
    return root;
}

//==============================================================================================================================================
StartProcessResult::StartProcessResult() {}

StartProcessResult::StartProcessResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

StartProcessResult::~StartProcessResult() {}

Json::Value StartProcessResult::toJson() const
{
    Json::Value root;
    return root;
}

//==============================================================================================================================================
StopProcessResult::StopProcessResult() {}

StopProcessResult::StopProcessResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

StopProcessResult::~StopProcessResult() {}

Json::Value StopProcessResult::toJson() const
{
    Json::Value root;
    return root;
}

//==============================================================================================================================================
GetProcessesResult::GetProcessesResult() {}

GetProcessesResult::GetProcessesResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

GetProcessesResult::~GetProcessesResult() {}

Json::Value GetProcessesResult::toJson() const
{
    Json::Value root(Json::arrayValue);
    for (const std::string &process : m_processes)
    {
        root.append(process);
    }
    return root;
}

//==============================================================================================================================================
GetProcessInfoResult::GetProcessInfoResult() {}

GetProcessInfoResult::GetProcessInfoResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

GetProcessInfoResult::~GetProcessInfoResult() {}

Json::Value GetProcessInfoResult::toJson() const
{
    Json::Value root;

    return root;
}

//==============================================================================================================================================
GetProcessConfigsResult::GetProcessConfigsResult() {}

GetProcessConfigsResult::GetProcessConfigsResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

GetProcessConfigsResult::~GetProcessConfigsResult() {}

Json::Value GetProcessConfigsResult::toJson() const
{
    Json::Value root(Json::arrayValue);
    for (const std::string &process : m_configs)
    {
        root.append(process);
    }
    return root;

}

//==============================================================================================================================================
GetProcessConfigResult::GetProcessConfigResult() {}

GetProcessConfigResult::GetProcessConfigResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

GetProcessConfigResult::~GetProcessConfigResult() {}

Json::Value GetProcessConfigResult::toJson() const
{
    Json::Value root(Json::arrayValue);
    for (const std::string &option : m_options)
    {
        root.append(option);
    }
    return root;
}

//==============================================================================================================================================
GetProcessOptionResult::GetProcessOptionResult() {}

GetProcessOptionResult::GetProcessOptionResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

GetProcessOptionResult::~GetProcessOptionResult() {}

class OptionValueToJsonVisitor
    : public boost::static_visitor<>
{
public:
    OptionValueToJsonVisitor(Json::Value &parent, const std::string &name) : 
        m_parent(parent), m_name(name)
    {
    }

    void operator()(const OptionSingleValue &v)
    {
        m_parent[m_name] = v;
    }

    void operator()(const OptionListValue &v)
    {
        Json::Value arr(Json::arrayValue);
        for (const OptionSingleValue &s : v)
        {
            arr.append(s);
        }
        
        m_parent[m_name] = arr;
    }

private:
    Json::Value &m_parent;
    const std::string m_name;
};

Json::Value GetProcessOptionResult::toJson() const
{
    Json::Value root;
    const OptionDesc &desc = m_option.get<0>();

    root["name"] = desc.get<0>();

    if (desc.get<1>().is_initialized())
    {
        OptionValueContainer defaultValue = desc.get<1>().get();
        OptionValueToJsonVisitor visitor(root, "default_value");
        boost::apply_visitor(visitor, defaultValue);
    }

    root["required"] = desc.get<2>() ? "true" : "false";

    root["list"] = desc.get<4>() ? "true" : "false";

    switch (desc.get<5>())
    {
    case OVT_NUMBER:
        root["type"] = "number";
        break;
    case OVT_STRING:
        root["type"] = "string";
        break;
    case OVT_DOMAIN:
        root["type"] = "domain";
        break;
    default:
        BOOST_ASSERT(false);
    }

    root["system"] = desc.get<6>() ? "true" : "false";

    if (m_option.get<1>().is_initialized())
    {
        const OptionValueContainer &value = m_option.get<1>().get();
        OptionValueToJsonVisitor visitor(root, "value");
        boost::apply_visitor(visitor, value);
    }

    if (desc.get<5>() == OVT_DOMAIN)
    {
        Json::Value arr(Json::arrayValue);
        for (const std::string &item : desc.get<7>())
        {
            arr.append(item);
        }
        root["domain"] = arr;
    }

    if (!m_option.get<2>().empty())
    {
        root["presentation"] = m_option.get<2>();
    }
    return root;
}
