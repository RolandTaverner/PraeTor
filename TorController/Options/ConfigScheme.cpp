#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "Options/ConfigScheme.h"
#include "Options/DefaultFormatter.h"

//-------------------------------------------------------------------------------------------------
ConfigScheme::ConfigScheme()
{
}

//-------------------------------------------------------------------------------------------------
ConfigScheme::~ConfigScheme()
{
}

//-------------------------------------------------------------------------------------------------
bool ConfigScheme::hasOption(const std::string &name) const
{
    return m_optionsDesc.find(name) != m_optionsDesc.end();
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
    const OptionConstraints &constraints,
    bool isList,
    OptionValueTag tag,
    bool isSystem,
    const OptionValueDomain &domain,
    const Tools::Configuration::ConfigurationView &format)
{
    if (m_optionsDesc.find(name) != m_optionsDesc.end())
    {
        throw OptionAlreadyRegistered("Option already registered.", name);
    }

    m_optionsDesc[name] = OptionDesc(name, defaultValue, required, constraints, isList, tag, isSystem, domain, format);
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
bool ConfigScheme::isList(const std::string &name) const
{
    return getOptionDesc(name).get<4>();
}

//-------------------------------------------------------------------------------------------------
bool ConfigScheme::isSystem(const std::string &name) const
{
    return getOptionDesc(name).get<6>();
}

//-------------------------------------------------------------------------------------------------
OptionValueContainer ConfigScheme::getDefaultValue(const std::string &name) const
{
    const OptionDesc &od = getOptionDesc(name);
    if (!od.get<1>().is_initialized())
    {
        throw OptionHasNoDefaultValue("Option has no default value.", name);
    }

    return od.get<1>().value();
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
void ConfigScheme::registerOption(const OptionDesc &od)
{
    const std::string name = od.get<0>();
    if (m_optionsDesc.find(name) != m_optionsDesc.end())
    {
        throw OptionAlreadyRegistered("Option already registered.", name);
    }

    m_optionsDesc[name] = od;
}

//-------------------------------------------------------------------------------------------------
const OptionValueDomain &ConfigScheme::getOptionValueDomain(const std::string &name) const
{
    OptionsDesc::const_iterator it = m_optionsDesc.find(name);
    if (it == m_optionsDesc.end())
    {
        throw OptionNotRegistered("Option is not registered.", name);
    }

    if (it->second.get<5>() != OVT_DOMAIN)
    {
        throw OptionValueHasNoDomain("Option is not domain value.", name);
    }

    return it->second.get<7>();
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
IConfigSchemePtr ConfigScheme::createFromConfig(const Tools::Configuration::ConfigurationView &conf)
{
    ConfigSchemePtr scheme(new ConfigScheme());
    
    BOOST_FOREACH(const Tools::Configuration::ConfigurationView &optConf, conf.getRangeOf("option"))
    {
        const OptionDesc od = createOptionDescFromConfig(optConf);
        scheme->registerOption(od);
    }

    return scheme;
}

typedef struct _OptionDescAttribute
{
    const char *name;
    bool isRequired;
    const char **values;
    unsigned valuesCount;
} OptionDescAttribute;

static const char *s_typeValues[] = { "number", "string", "domainvalue" };
static const char *s_requiredValues[] = { "no", "yes" };
static const char *s_listValues[] = { "no", "yes" };
static const char *s_systemValues[] = { "no", "yes" };

static const OptionDescAttribute s_optionAttributes[] = 
{
    { "name",     true,  NULL,             0 },
    { "type",     true,  s_typeValues,     sizeof(s_typeValues) / sizeof(*s_typeValues) },
    { "required", false, s_requiredValues, sizeof(s_requiredValues) / sizeof(*s_requiredValues) },
    { "list",     false, s_listValues,     sizeof(s_listValues) / sizeof(*s_listValues) },
    { "system",   false, s_systemValues,   sizeof(s_systemValues) / sizeof(*s_systemValues) },
};

typedef struct _OptionTypeToTag
{
    OptionValueTag tag;
    const char *type;
} OptionTypeToTag;

static const OptionTypeToTag s_optionTypeToTag[] = 
{
    { OVT_NUMBER, s_typeValues[0] },
    { OVT_STRING, s_typeValues[1] },
    { OVT_DOMAIN, s_typeValues[2] }
};

//-------------------------------------------------------------------------------------------------
OptionValueTag optionTypeToTag(const std::string &type)
{
    for (unsigned i = 0; i < sizeof(s_optionTypeToTag) / sizeof(s_optionTypeToTag[0]); ++i)
    {
        if (boost::equal(type, std::string(s_optionTypeToTag[i].type)))
            return s_optionTypeToTag[i].tag;
    }

    BOOST_ASSERT_MSG(false, "Unknown type value.");
    return OVT_STRING;
}

//-------------------------------------------------------------------------------------------------
OptionDesc ConfigScheme::createOptionDescFromConfig(const Tools::Configuration::ConfigurationView &optConf)
{
    // Check option attributes
    for (unsigned i = 0; i < (sizeof(s_optionAttributes) / sizeof(s_optionAttributes[0])); ++i)
    {
        const std::string attrName = s_optionAttributes[i].name;
        const bool isRequiredAttr = s_optionAttributes[i].isRequired;
        
        if (isRequiredAttr && !optConf.hasAttr("", attrName))
        {
            throw OptionDefinitionError("Required attribute not found.", "");
        }

        if (s_optionAttributes[i].values != NULL && s_optionAttributes[i].valuesCount > 0u && optConf.hasAttr("", attrName))
        {
            const std::string attrValue = optConf.getAttr("", attrName);
            bool found = false;
            for (unsigned j = 0; j < s_optionAttributes[i].valuesCount; ++j)
            {
                if (boost::iequals(attrValue, s_optionAttributes[i].values[j]))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw OptionDefinitionError("Attribute value not in list.", "");
            }
        }
    }

    const std::string name = optConf.getAttr("", "name");
    const std::string type = boost::to_lower_copy(optConf.getAttr("", "type"));
    const bool isRequired = boost::iequals(optConf.getAttr<std::string>("", "required", std::string("no")), "yes");
    const bool isList = boost::iequals(optConf.getAttr<std::string>("", "list", std::string("no")), "yes");
    const bool isSystem = boost::iequals(optConf.getAttr<std::string>("", "system", std::string("no")), "yes");

    OptionValueType defaultValue;
    if (optConf.exists("default"))
    {
        if (!isList && optConf.getCountOf("default") > 1u)
        {
            throw OptionDefinitionError("More than one default value specified for single-value option.", name);
        }

        if (!isList)
        {
            defaultValue = OptionValueContainer(optConf.get("default.value"));
        }
        else
        {
            OptionListValue defaultValues;
            BOOST_FOREACH(const Tools::Configuration::ConfigurationView &defaultItem, optConf.getRangeOf("default.value"))
            {
                defaultValues.push_back(defaultItem.get(""));
            }
            defaultValue = OptionValueContainer(defaultValues);
        }
    }

    const OptionValueTag typeTag = optionTypeToTag(type);

    OptionValueDomain domain;
    if (typeTag == OVT_DOMAIN)
    {
        BOOST_FOREACH(const Tools::Configuration::ConfigurationView &domainItem, optConf.getRangeOf("type.domain.value"))
        {
            domain.insert(domainItem.get(""));
        }

        if (domain.empty())
        {
            throw OptionDefinitionError("Empty domain not allowed.", name);
        }
    }
    // TODO: constraints and default value check
    OptionConstraints constraints;

    return OptionDesc(name, defaultValue, isRequired, constraints, isList, typeTag, isSystem, domain, optConf.branch("format"));
}

//-------------------------------------------------------------------------------------------------
std::string ConfigScheme::formatOption(const std::string &name, const OptionValueContainer &value, ISubstitutorPtr substitutorPtr) const
{
    OptionsDesc::const_iterator it = m_optionsDesc.find(name);
    if (it == m_optionsDesc.end())
    {
        throw OptionNotRegistered("Option is not registered.", name);
    }

    std::ostringstream out;

    OptionsFormatters::const_iterator itFormatter = m_formatters.find(name);
    if (itFormatter != m_formatters.end())
    {
        itFormatter->second->format(Option(name, value), it->second, out);
    }
    else
    {
        DefaultFormatter().format(Option(name, value), it->second, out);
    }

    const std::string formattedOption = out.str();
    std::string result = formattedOption;

    boost::regex r("\\%([\\d\\w]+)\\%");

    boost::sregex_token_iterator iter(formattedOption.begin(), formattedOption.end(), r, 1);
    boost::sregex_token_iterator end;

    for (; iter != end; ++iter) 
    {
        const std::string token = *iter;
        if (substitutorPtr->hasSubstitute(token))
        {
            boost::algorithm::replace_all(result, "%" + token + "%", substitutorPtr->substituteValue(token));
        }
        else
        {
        // TODO: throw?
        }
    }
    return result;
}

//-------------------------------------------------------------------------------------------------
void ConfigScheme::setFormatter(const std::string &name, IFormatterPtr formatterPtr)
{
    OptionsDesc::const_iterator it = m_optionsDesc.find(name);
    if (it == m_optionsDesc.end())
    {
        throw OptionNotRegistered("Option is not registered.", name);
    }

    m_formatters[name] = formatterPtr;
}