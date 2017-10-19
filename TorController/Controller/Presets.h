#pragma once

#include <map>
#include <string>

#include <Tools/Configuration/ConfigurationView.h>

#include "Options/AbstractCollection.h"
#include "Process/ProcessConfiguration.h"

typedef std::map<std::string, ProcessConfiguration> PresetGroup;

class Presets : public AbstractCollection<PresetGroup>
{
public:
    Presets();
    virtual ~Presets();

    void load(const Tools::Configuration::ConfigurationView &presetsConf,
        const Tools::Configuration::ConfigurationView &processesConf);

private:
    typedef std::map<std::string, PresetGroup> PresetsMap;
    PresetsMap m_presetsStorage;

    // AbstractCollection<PresetGroup> implementation
    CollectionType::Element *begin() const override;
    CollectionType::Element *end() const override;
    CollectionType::Element *next(const CollectionType::Element *current) const override;
    const CollectionType::CollectionValueType &dereference(const CollectionType::Element *current) const override;

    class PresetElement : public CollectionType::Element
    {
    public:
        PresetElement(const CollectionType *storage, PresetsMap::const_iterator it) :
            CollectionType::Element(storage), m_iterator(it)
        {
        }

        const PresetsMap::const_iterator &getIterator() const
        {
            return m_iterator;
        }

        CollectionType::Element *clone() const override
        {
            return new PresetElement(getCollection(), m_iterator);
        }

        bool equals(const Element *rhs) const override
        {
            BOOST_ASSERT(rhs != NULL);
            BOOST_ASSERT(this->getCollection() == rhs->getCollection());
            BOOST_ASSERT(dynamic_cast<const PresetElement*>(rhs) != NULL);
            const PresetElement *other = dynamic_cast<const PresetElement*>(rhs);
            return this->getIterator() == other->getIterator();
        }

    private:
        PresetsMap::const_iterator m_iterator;
    };

};

