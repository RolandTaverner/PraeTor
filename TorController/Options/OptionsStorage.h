#pragma once

#include <map>

#include "Options/IConfigScheme.h"
#include "Options/IOptionsStorage.h"

class OptionsStorage :
    public IOptionsStorage
{
public:
    explicit OptionsStorage(IConfigSchemePtr scheme);
    virtual ~OptionsStorage();
    
    // IOptionsStorage
    void setValue(const std::string &name, const OptionValueType &value) override; /* throws */
    
    const OptionValueType &getValue(const std::string &name) const override; /* throws */

    bool hasValue(const std::string &name) const override;

    IConfigSchemePtr getScheme() const override;

    std::string formatOption(const std::string &name) const override;

private:
    typedef std::map<std::string, Option> Options;
    Options m_options;
    IConfigSchemePtr m_scheme;

    // AbstractCollection<Option> implementation
    CollectionType::Element *begin() const override;
    CollectionType::Element *end() const override;
    CollectionType::Element *next(const CollectionType::Element *current) const override;
    const CollectionType::CollectionValueType &dereference(const CollectionType::Element *current) const override;
    
    class OptionsStorageElement : public CollectionType::Element
    {
    public:
        OptionsStorageElement(const CollectionType *storage, Options::const_iterator it) :
            CollectionType::Element(storage), m_iterator(it)
        {
        }

        const Options::const_iterator &getIterator() const
        {
            return m_iterator;
        }

        CollectionType::Element *clone() const override
        {
            return new OptionsStorageElement(getCollection(), m_iterator);
        }

        bool equals(const Element *rhs) const override
        {
            BOOST_ASSERT(rhs != NULL);
            BOOST_ASSERT(this->getCollection() == rhs->getCollection());
            BOOST_ASSERT(dynamic_cast<const OptionsStorageElement*>(rhs) != NULL);
            const OptionsStorageElement *other = dynamic_cast<const OptionsStorageElement*>(rhs);
            return this->getIterator() == other->getIterator();
        }

    private:
        Options::const_iterator m_iterator;
    };
};

