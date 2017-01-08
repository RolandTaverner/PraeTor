#include <boost/assert.hpp>
#include <boost/foreach.hpp>

#include "Options/ConfigScheme.h"

//-------------------------------------------------------------------------------------------------
ConfigScheme::ConfigScheme()
{
}

//-------------------------------------------------------------------------------------------------
ConfigScheme::~ConfigScheme()
{
}

//-------------------------------------------------------------------------------------------------
void ConfigScheme::checkOption(const Option &opt)
{
    const OptionDesc &desc = getOptionDesc(opt.name());
    const OptionConstraints constraints = desc.get<3>();
    OptionConstraints failedConstraints;

    BOOST_FOREACH(IOptionConstraintPtr constraint, constraints)
    {
        if (!constraint->isValid(opt))
        {
            failedConstraints.push_back(constraint);
        }
    }

    if (!failedConstraints.empty())
    {
        throw ConstraintCheckFailed("Constraints check failed.", opt.name(), failedConstraints);
    }
}

//-------------------------------------------------------------------------------------------------
void ConfigScheme::registerOption(const std::string &name,
    const OptionValueType &defaultValue,
    bool required,
    const OptionConstraints &constraints)
{
    if (m_optionsDesc.find(name) != m_optionsDesc.end())
    {
        throw OptionAlreadyRegistered("Option already registered.", name);
    }

    m_optionsDesc[name] = OptionDesc(name, defaultValue, required, constraints);
}

//-------------------------------------------------------------------------------------------------
bool ConfigScheme::isRequired(const std::string &name) const
{
    return getOptionDesc(name).get<2>();
}

//-------------------------------------------------------------------------------------------------
bool ConfigScheme::hasDefaultValue(const std::string &name) const
{
    return getOptionDesc(name).get<1>().is_initialized();
}

//-------------------------------------------------------------------------------------------------
const OptionDesc &ConfigScheme::getOptionDesc(const std::string &name) const
{
    OptionsDesc::const_iterator it = m_optionsDesc.find(name);
    if (it == m_optionsDesc.end())
    {
        throw OptionNotRegistered("Option is not registered.", name);
    }

    return it->second;
}

//-------------------------------------------------------------------------------------------------
ConfigScheme::CollectionType::Element *ConfigScheme::begin() const
{
    return new ConfigSchemeElement(this, m_optionsDesc.cbegin());
}

//-------------------------------------------------------------------------------------------------
ConfigScheme::CollectionType::Element *ConfigScheme::end() const
{
    return new ConfigSchemeElement(this, m_optionsDesc.cend());
}

//-------------------------------------------------------------------------------------------------
ConfigScheme::CollectionType::Element *ConfigScheme::next(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const ConfigSchemeElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const ConfigSchemeElement*>(current)->getIterator() != m_optionsDesc.end());

    const ConfigSchemeElement *element = dynamic_cast<const ConfigSchemeElement*>(current);
    OptionsDesc::const_iterator iterator = element->getIterator();

    return new ConfigSchemeElement(this, ++iterator);
}

//-------------------------------------------------------------------------------------------------
const ConfigScheme::CollectionType::CollectionValueType &ConfigScheme::dereference(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const ConfigSchemeElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const ConfigSchemeElement*>(current)->getIterator() != m_optionsDesc.end());

    return dynamic_cast<const ConfigSchemeElement*>(current)->getIterator()->second;
}

//-------------------------------------------------------------------------------------------------
IConfigSchemePtr ConfigScheme::CreateFromConfig(const Tools::Configuration::ConfigurationView &conf)
{

}