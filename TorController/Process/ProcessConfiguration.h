#pragma once

#include <map>
#include <stdexcept>
#include <string>

#include "Options/AbstractCollection.h"
#include "Options/IOptionsStorage.h"

class ProcessConfiguration : public AbstractCollection<IOptionsStoragePtr>
{
public:
    ProcessConfiguration();
    virtual ~ProcessConfiguration();

    bool hasStorage(const std::string &name) const;

    IOptionsStoragePtr getStorage(const std::string &name);

    IOptionsStorageConstPtr getStorage(const std::string &name) const;

    void addStorage(const std::string &name, IOptionsStoragePtr storagePtr);

private:
    typedef std::map<std::string, IOptionsStoragePtr> OptionsStorages;

    OptionsStorages m_storages;

    // AbstractCollection<OptionDesc> implementation
    CollectionType::Element *begin() const override;
    CollectionType::Element *end() const override;
    CollectionType::Element *next(const CollectionType::Element *current) const override;
    const CollectionType::CollectionValueType &dereference(const CollectionType::Element *current) const override;

    class ConfigSchemeElement : public CollectionType::Element
    {
    public:
        ConfigSchemeElement(const CollectionType *storage, OptionsStorages::const_iterator it) :
            CollectionType::Element(storage), m_iterator(it)
        {
        }

        const OptionsStorages::const_iterator &getIterator() const
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
        OptionsStorages::const_iterator m_iterator;
    };

};

