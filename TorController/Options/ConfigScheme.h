#pragma once

#include <map>
#include <string>

#include "Tools/Configuration/ConfigurationView.h"

#include "Options/IConfigScheme.h"
#include "Options/IFormatter.h"

class ConfigScheme :
    public IConfigScheme
{
public:
    ConfigScheme();
    virtual ~ConfigScheme();

    // IConfigScheme implementation
    bool hasOption(const std::string &name) const override;

    void checkOption(const Option &opt) override; /* throws */

    void registerOption(const std::string &name,
        const OptionValueType &defaultValue,
        bool required,
        const OptionConstraints &constraints,
        bool isList,
        OptionValueTag tag,
        bool isSystem,
        const OptionValueDomain &domain,
        const Tools::Configuration::ConfigurationView &format) override;  /* throws */

    bool isRequired(const std::string &name) const override; /* throws */

    bool hasDefaultValue(const std::string &name) const override; /* throws */

    bool isList(const std::string &name) const override; /* throws */

    bool isSystem(const std::string &name) const override; /* throws */

    OptionValueContainer getDefaultValue(const std::string &name) const override; /* throws */

    const OptionDesc &getOptionDesc(const std::string &name) const override; /* throws */

    const OptionValueDomain &getOptionValueDomain(const std::string &name) const override; /* throws */

    std::string formatOption(const std::string &name, const OptionValueContainer &value) const override; /* throws */

    void setFormatter(const std::string &name, IFormatterPtr formatterPtr) override; /* throws */
    // End of IConfigScheme implementation

    static IConfigSchemePtr createFromConfig(const Tools::Configuration::ConfigurationView &conf);

protected:
    void registerOption(const OptionDesc &od);

private:
    typedef std::map<std::string, OptionDesc> OptionsDesc;
    OptionsDesc m_optionsDesc;
    
    typedef std::map<std::string, IFormatterPtr> OptionsFormatters;
    OptionsFormatters m_formatters;

    static OptionDesc createOptionDescFromConfig(const Tools::Configuration::ConfigurationView &optConf);

    // AbstractCollection<OptionDesc> implementation
    CollectionType::Element *begin() const override;
    CollectionType::Element *end() const override;
    CollectionType::Element *next(const CollectionType::Element *current) const override;
    const CollectionType::CollectionValueType &dereference(const CollectionType::Element *current) const override;

    class ConfigSchemeElement : public CollectionType::Element
    {
    public:
        ConfigSchemeElement(const CollectionType *storage, OptionsDesc::const_iterator it) :
            CollectionType::Element(storage), m_iterator(it)
        {
        }

        const OptionsDesc::const_iterator &getIterator() const
        {
            return m_iterator;
        }

        CollectionType::Element *clone() const override
        {
            return new ConfigSchemeElement(getCollection(), m_iterator);
        }

        bool equals(const Element *rhs) const override
        {
            BOOST_ASSERT(rhs != NULL);
            BOOST_ASSERT(this->getCollection() == rhs->getCollection());
            BOOST_ASSERT(dynamic_cast<const ConfigSchemeElement*>(rhs) != NULL);
            const ConfigSchemeElement *other = dynamic_cast<const ConfigSchemeElement*>(rhs);
            return this->getIterator() == other->getIterator();
        }

    private:
        OptionsDesc::const_iterator m_iterator;
    };

};


typedef boost::shared_ptr<ConfigScheme> ConfigSchemePtr;