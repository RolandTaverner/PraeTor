#include <boost/assert.hpp>

#include "Options/ConfigScheme.h"
#include "Options/OptionsStorage.h"
#include "Controller/ControllerErrors.h"
#include "Controller/Presets.h"
#include "Process/ProcessErrors.h"
#include "Options/OptionErrors.h"

//-------------------------------------------------------------------------------------------------
Presets::Presets()
{
}

//-------------------------------------------------------------------------------------------------
Presets::~Presets()
{
}

//-------------------------------------------------------------------------------------------------
void Presets::load(const Tools::Configuration::ConfigurationView &presetsConf,
    const Tools::Configuration::ConfigurationView &processesConf)
{
    // Load processes
    typedef std::map<std::string, IConfigSchemePtr> SchemeMap;
    typedef std::map<std::string, SchemeMap> ProcessSchemes;
    ProcessSchemes processesSchemes;

    for (const Tools::Configuration::ConfigurationView &processConf : processesConf.getRangeOf("process"))
    {
        const std::string processName = processConf.getAttr("", "name");

        for (const Tools::Configuration::ConfigurationView &schemeConf: processConf.getRangeOf("options.scheme"))
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
    for (const Tools::Configuration::ConfigurationView &presetConf : presetsConf.getRangeOf("preset"))
    {
        const std::string presetGroupName = presetConf.getAttr("", "name");
        PresetGroupConfig group;
        for (const Tools::Configuration::ConfigurationView &processConf : presetConf.getRangeOf("process"))
        {
            const std::string processName = processConf.getAttr("", "name");
            ProcessSchemes::const_iterator itPs = processesSchemes.find(processName);
            if (itPs == processesSchemes.end())
            {
                throw ControllerError(makeErrorCode(ControllerErrors::processNotFound), "Can't load presets for process " + processName);
            }
            const SchemeMap &processSchemes = itPs->second;

            ProcessConfiguration processConfig;

            for (const Tools::Configuration::ConfigurationView &schemeConf : processConf.getRangeOf("scheme"))
            {
                const std::string schemeName = schemeConf.getAttr("", "name");
                SchemeMap::const_iterator itScheme = processSchemes.find(schemeName);
                if (itScheme == processSchemes.end())
                {
                    throw ProcessError(makeErrorCode(ProcessErrors::noSuchStorage), "Can't load presets for process " + processName + " storage " + schemeName);
                }
                IConfigSchemePtr schemePtr = itScheme->second;
                IOptionsStoragePtr storage(new OptionsStorage(schemePtr, false));
                
                for (const Tools::Configuration::ConfigurationView &optionConf : schemeConf.getRangeOf("option"))
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
                        for (const Tools::Configuration::ConfigurationView &item : optionConf.getRangeOf("value"))
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
            } // for (const Tools::Configuration::ConfigurationView &schemeConf : processConf.getRangeOf("scheme"))

            if (!processConfig.getRange().empty())
            {
                group[processName] = processConfig;
            }
        } // for (const Tools::Configuration::ConfigurationView &processConf : presetConf.getRangeOf("process"))
        
        if (!group.empty())
        {
            m_presetsStorage[presetGroupName] = PresetGroup(presetGroupName, group);
        }
    } // for (const Tools::Configuration::ConfigurationView &presetConf : presetsConf.getRangeOf("preset"))
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
