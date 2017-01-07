#pragma once

#include <map>
#include <string>

#include "Options/IConfigScheme.h"

class ConfigScheme :
    public IConfigScheme
{
public:
    ConfigScheme();
    virtual ~ConfigScheme();

    // IConfigScheme
    void checkOption(const Option &opt) override; /* throws */

    void registerOption(const std::string &name,
        const OptionValueType &defaultValue,
        bool required,
        const OptionConstraints &constraints) override;  /* throws */

    bool isRequired(const std::string &name) const override; /* throws */

    bool hasDefaultValue(const std::string &name) const override; /* throws */

    const OptionDesc &getOptionDesc(const std::string &name) const override; /* throws */

private:
    typedef std::map<std::string, OptionDesc> OptionsDesc;
    OptionsDesc m_optionsDesc;

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

