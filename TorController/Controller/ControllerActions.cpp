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
    if (getError())
    {
        root["status"] = "FAIL";
    }
    else
    {
        root["status"] = "OK";
    }
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

    root["exit_code"] = m_exitStatus.exitCode;
    root["unexpected_exit"] = m_exitStatus.unexpectedExit;

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

    root["name"] = m_name;

    switch (m_state)
    {
    case ProcessState::Running:
        root["status"] = "running";
        break;
    case ProcessState::Starting:
        root["status"] = "starting";
        break;
    case ProcessState::Stopping:
        root["status"] = "stopping";
        break;
    case ProcessState::Stopped:
        root["status"] = "stopped";
        break;
    default:
        root["status"] = "unknown";
    }

    Json::Value arr(Json::arrayValue);
    for (const std::string &item : m_configs)
    {
        arr.append(item);
    }
    root["configs"] = arr;

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
    Json::Value root;
    root["name"] = m_name;

    Json::Value arr(Json::arrayValue);
    for (const std::string &option : m_options)
    {
        arr.append(option);
    }
    root["options"] = arr;
    return root;
}

//==============================================================================================================================================
ProcessOptionResult::ProcessOptionResult() {}

ProcessOptionResult::ProcessOptionResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

ProcessOptionResult::~ProcessOptionResult() {}

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

Json::Value ProcessOptionResult::toJson() const
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

    root["required"] = desc.get<2>();

    root["list"] = desc.get<4>();

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

    root["system"] = desc.get<6>();

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

//==============================================================================================================================================
PresetGroupsResult::PresetGroupsResult() {}

PresetGroupsResult::PresetGroupsResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

PresetGroupsResult::~PresetGroupsResult() {}

Json::Value PresetGroupsResult::toJson() const
{
    Json::Value root(Json::arrayValue);
    for (const std::string &groupName : m_presetGroups)
    {
        root.append(groupName);
    }

    return root;
}

//==============================================================================================================================================
ApplyPresetGroupResult::ApplyPresetGroupResult() {}

ApplyPresetGroupResult::ApplyPresetGroupResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

ApplyPresetGroupResult::~ApplyPresetGroupResult() {}

Json::Value ApplyPresetGroupResult::toJson() const
{
    Json::Value root;

    return root;
}

//==============================================================================================================================================
PresetsResult::PresetsResult() {}

PresetsResult::PresetsResult(const ErrorCode &ec) :
    ActionResult(ec)
{
}

PresetsResult::~PresetsResult() {}

class OptionValueToJsonVisitorExplicitArray
    : public boost::static_visitor<>
{
public:
    OptionValueToJsonVisitorExplicitArray(Json::Value &parent, const std::string &name, const std::string &arrayName) :
        m_parent(parent), m_name(name), m_arrayName(arrayName)
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

        m_parent[m_arrayName] = arr;
    }

private:
    Json::Value &m_parent;
    const std::string m_name;
    const std::string m_arrayName;
};

Json::Value PresetsResult::toJson() const
{
    Json::Value root(Json::arrayValue);

    for (ProcessOptions::const_iterator itProcess = m_processOptions.begin(); itProcess != m_processOptions.end(); ++itProcess)
    {
        const std::string &processName = itProcess->first;
        Json::Value processNode;
        processNode["process_id"] = processName;
        Json::Value configsNode(Json::arrayValue);
        const SchemeOptions &schemes = itProcess->second;
        for (SchemeOptions::const_iterator itScheme = schemes.begin(); itScheme != schemes.end(); ++itScheme)
        {
            const std::string &configName = itScheme->first;
            Json::Value schemeNode;
            schemeNode["config_name"] = configName;
            Json::Value optsNode(Json::arrayValue);
            const Options &opts = itScheme->second;
            for (Options::const_iterator itOption = opts.begin(); itOption != opts.end(); ++ itOption)
            {
                Json::Value optionNode;
                optionNode["name"] = itOption->name();

                if (itOption->value().is_initialized())
                {
                    const OptionValueContainer &value = itOption->value().get();
                    OptionValueToJsonVisitorExplicitArray visitor(optionNode, "value", "array_value");
                    boost::apply_visitor(visitor, value);
                }

                optsNode.append(optionNode);
            }
            schemeNode["options"] = optsNode;
            configsNode.append(schemeNode);
        }
        processNode["configs"] = configsNode;
        root.append(processNode);
    }

    return root;
}