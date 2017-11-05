#include <boost/assert.hpp>
#include <boost/variant/static_visitor.hpp>

#include <Tools/Configuration/Configuration.h>

#include "Options/ConfigScheme.h"
#include "Options/OptionsStorage.h"
#include "Controller/ControllerErrors.h"
#include "Controller/Presets.h"
#include "Process/ProcessErrors.h"
#include "Options/OptionErrors.h"

namespace tc = Tools::Configuration;

//-------------------------------------------------------------------------------------------------
Presets::Presets()
{
}

//-------------------------------------------------------------------------------------------------
Presets::~Presets()
{
}

//-------------------------------------------------------------------------------------------------
void Presets::load(const tc::ConfigurationView &presetsConf,
    const tc::ConfigurationView &processesConf, bool keepEmpty)
{
    // Load processes
    ProcessSchemes processesSchemes;

    for (const tc::ConfigurationView &processConf : processesConf.getRangeOf("process"))
    {
        const std::string processName = processConf.getAttr("", "name");

        for (const tc::ConfigurationView &schemeConf: processConf.getRangeOf("options.scheme"))
        {
            const std::string schemeName = schemeConf.getAttr("", "name");
            IConfigSchemePtr scheme = ConfigScheme::createFromConfig(schemeConf);
            if (!scheme->getRange().empty())
            {
                processesSchemes[processName][schemeName] = scheme;
            }
        }
    }

    // Load presets
    for (const tc::ConfigurationView &presetConf : presetsConf.getRangeOf("preset"))
    {
        const std::string presetGroupName = presetConf.getAttr("", "name");
        PresetGroupConfig group = loadPresetGroup(presetConf, processesSchemes, keepEmpty);
        
        if (!group.empty())
        {
            m_presetsStorage[presetGroupName] = PresetGroup(presetGroupName, group);
        }
    } // for (const tc::ConfigurationView &presetConf : presetsConf.getRangeOf("preset"))
}

//-------------------------------------------------------------------------------------------------
PresetGroupConfig Presets::loadPresetGroup(const tc::ConfigurationView &presetConf,
    const Presets::ProcessSchemes &processesSchemes, bool keepEmpty) const
{
    PresetGroupConfig group;

    for (const tc::ConfigurationView &processConf : presetConf.getRangeOf("process"))
    {
        const std::string processName = processConf.getAttr("", "name");
        ProcessSchemes::const_iterator itPs = processesSchemes.find(processName);
        if (itPs == processesSchemes.end())
        {
            throw ControllerError(makeErrorCode(ControllerErrors::processNotFound), "Can't load presets for process " + processName);
        }
        const SchemeMap &processSchemes = itPs->second;

        ProcessConfiguration processConfig;

        for (const tc::ConfigurationView &schemeConf : processConf.getRangeOf("scheme"))
        {
            const std::string schemeName = schemeConf.getAttr("", "name");
            SchemeMap::const_iterator itScheme = processSchemes.find(schemeName);
            if (itScheme == processSchemes.end())
            {
                throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage), "Can't load presets for process " + processName + " storage " + schemeName);
            }
            IConfigSchemePtr schemePtr = itScheme->second;
            IOptionsStoragePtr storage(new OptionsStorage(schemePtr, false));

            for (const tc::ConfigurationView &optionConf : schemeConf.getRangeOf("option"))
            {
                const std::string optionName = optionConf.getAttr("", "name");
                if (!schemePtr->hasOption(optionName))
                {
                    throw ProcessError(makeErrorCode(ProcessErrors::noSuchOption),
                        "Can't load presets for process " + processName + ", storage " + schemeName + ", option " + optionName);
                }

                Option option;
                if (schemePtr->isList(optionName))
                {
                    OptionListValue value;
                    for (const tc::ConfigurationView &item : optionConf.getRangeOf("value"))
                    {
                        value.push_back(item.get(""));
                    }

                    option = Option(optionName, OptionValueContainer(value));
                }
                else
                {
                    if (optionConf.getCountOf("value") > 1)
                    {
                        throw ProcessError(makeErrorCode(OptionErrors::assigningListToSingleValue),
                            "Can't load presets for process " + processName + ", storage " + schemeName + ", option " + optionName
                            + ": can't assign list to single value option");
                    }

                    const OptionSingleValue value = optionConf.get("value", std::string(""));
                    option = Option(optionName, OptionValueContainer(value));
                }

                schemePtr->checkOption(option);
                storage->setValue(option.name(), option.value());
            }

            if (!storage->getRange().empty())
            {
                processConfig.addStorage(schemeName, storage);
            }
        } // for (const tc::ConfigurationView &schemeConf : processConf.getRangeOf("scheme"))

        if (!processConfig.getRange().empty())
        {
            group[processName] = processConfig;
        }
    } // for (const tc::ConfigurationView &processConf : presetConf.getRangeOf("process"))

    if (keepEmpty)
    {
        addAllProcessConfigs(group, processesSchemes);
    }
    
    return group;
}

//-------------------------------------------------------------------------------------------------
void Presets::addAllProcessConfigs(PresetGroupConfig &group, const Presets::ProcessSchemes &processesSchemes)
{
    for (const ProcessSchemes::value_type &ps : processesSchemes)
    {
        const std::string processName = ps.first;
        const SchemeMap &processSchemes = ps.second;
        if (group.find(processName) == group.end())
        {
            group[processName] = ProcessConfiguration();
        }

        ProcessConfiguration &processConfig = group.find(processName)->second;

        for (const SchemeMap::value_type &s : processSchemes)
        {
            const std::string schemeName = s.first;
            if (!processConfig.hasStorage(schemeName))
            {
                IConfigSchemePtr schemePtr = s.second;
                IOptionsStoragePtr storage(new OptionsStorage(schemePtr, false));
                processConfig.addStorage(schemeName, storage);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
Presets::CollectionType::Element *Presets::begin() const
{
    return new PresetElement(this, m_presetsStorage.cbegin());
}

//-------------------------------------------------------------------------------------------------
Presets::CollectionType::Element *Presets::end() const
{
    return new PresetElement(this, m_presetsStorage.cend());
}

//-------------------------------------------------------------------------------------------------
Presets::CollectionType::Element *Presets::next(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const PresetElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const PresetElement*>(current)->getIterator() != m_presetsStorage.end());

    const PresetElement *element = dynamic_cast<const PresetElement*>(current);
    PresetsMap::const_iterator iterator = element->getIterator();

    return new PresetElement(this, ++iterator);
}

//-------------------------------------------------------------------------------------------------
const Presets::CollectionType::CollectionValueType &Presets::dereference(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const PresetElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const PresetElement*>(current)->getIterator() != m_presetsStorage.end());

    return dynamic_cast<const PresetElement*>(current)->getIterator()->second;
}

//-------------------------------------------------------------------------------------------------
const PresetGroup &Presets::getPresets(const std::string &groupName) const
{
    PresetsMap::const_iterator it = m_presetsStorage.find(groupName);
    if (it == m_presetsStorage.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::presetsNotFound), groupName);
    }
    
    return it->second;
}

//-------------------------------------------------------------------------------------------------
void Presets::setOption(const std::string &groupName, const std::string &processName, const std::string &configName, const Option &option)
{
    PresetsMap::iterator itGroup = m_presetsStorage.find(groupName);
    if (itGroup == m_presetsStorage.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::presetsNotFound), groupName);
    }
    
    PresetGroup &pg = itGroup->second;
    PresetGroupConfig::iterator itProcess = pg.second.find(processName);
    if (itProcess == pg.second.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound), processName);
    }

    ProcessConfiguration &pc = itProcess->second;
    if (!pc.hasStorage(configName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage), configName);
    }

    IOptionsStoragePtr storagePtr = pc.getStorage(configName);
    storagePtr->setValue(option.name(), option.value());
}

//-------------------------------------------------------------------------------------------------
class OptionValueToConfVisitor
    : public boost::static_visitor<>
{
public:
    OptionValueToConfVisitor(tc::Configuration &parent) :
        m_parent(parent)
    {
    }

    void operator()(const OptionSingleValue &v)
    {
        m_parent.set("value", v);
    }

    void operator()(const OptionListValue &v)
    {
        for (const OptionSingleValue &s : v)
        {
            m_parent.add("value").set("", s);
        }
    }

private:
    tc::Configuration &m_parent;
};

tc::ConfigurationView Presets::toConfiguration() const
{
    tc::Configuration conf;
    tc::Configuration presets = conf.add("presets");

    for (const PresetsMap::value_type &pg : m_presetsStorage)
    {
        tc::Configuration presetGroupConf = presets.add("preset");
        presetGroupConf.setAttr("", "name", pg.first);
        
        for (const PresetGroupConfig::value_type &process : pg.second.second)
        {
            const std::string &processName = process.first;
            const ProcessConfiguration &pc = process.second;
            tc::Configuration processConf = presetGroupConf.add("process");
            processConf.setAttr("", "name", processName);

            std::list<std::string> storageNames;
            pc.getStorages(storageNames);
            for (const std::string &storageName : storageNames)
            {
                tc::Configuration storageConf = processConf.add("scheme");
                storageConf.setAttr("", "name", storageName);
                IOptionsStorageConstPtr storagePtr = pc.getStorage(storageName);
                for (const Option &o: storagePtr->getRange())
                {
                    tc::Configuration optionConf = storageConf.add("option");
                    optionConf.setAttr("", "name", o.name());
                    if (o.value().is_initialized())
                    {
                        OptionValueToConfVisitor visitor(optionConf);
                        boost::apply_visitor(visitor, o.value().get());
                    }
                } //for (const Option &o: storagePtr->getRange())
            } // for (const std::string &storageName : storageNames)
        } // for (const PresetGroupConfig::value_type &process : pg.second.second)
    } // for (const PresetsMap::value_type &pg : m_presetsStorage)

    return conf;
}

//-------------------------------------------------------------------------------------------------
Presets Presets::createTemplate(const std::string &presetGroupName, const tc::ConfigurationView &processesConf)
{
    Presets result;

    // Load processes
    ProcessSchemes processesSchemes;

    for (const tc::ConfigurationView &processConf : processesConf.getRangeOf("process"))
    {
        const std::string processName = processConf.getAttr("", "name");

        for (const tc::ConfigurationView &schemeConf : processConf.getRangeOf("options.scheme"))
        {
            const std::string schemeName = schemeConf.getAttr("", "name");
            IConfigSchemePtr scheme = ConfigScheme::createFromConfig(schemeConf);
            if (!scheme->getRange().empty())
            {
                processesSchemes[processName][schemeName] = scheme;
            }
        }
    }

    PresetGroupConfig group;
    addAllProcessConfigs(group, processesSchemes);

    if (!group.empty())
    {
        result.m_presetsStorage[presetGroupName] = PresetGroup(presetGroupName, group);
    }

    return result;
}

//-------------------------------------------------------------------------------------------------
void Presets::removeOption(const std::string &groupName, const std::string &processName, const std::string &configName, const std::string &optionName)
{
    PresetsMap::iterator itGroup = m_presetsStorage.find(groupName);
    if (itGroup == m_presetsStorage.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::presetsNotFound), groupName);
    }

    PresetGroup &pg = itGroup->second;
    PresetGroupConfig::iterator itProcess = pg.second.find(processName);
    if (itProcess == pg.second.end())
    {
        throw ControllerError(makeErrorCode(ControllerErrors::processNotFound), processName);
    }

    ProcessConfiguration &pc = itProcess->second;
    if (!pc.hasStorage(configName))
    {
        throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage), configName);
    }

    IOptionsStoragePtr storagePtr = pc.getStorage(configName);
    storagePtr->removeValue(optionName);
}