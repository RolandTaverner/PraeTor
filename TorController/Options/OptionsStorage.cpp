#include <boost/assert.hpp>

#include "Options/OptionsStorage.h"

//-------------------------------------------------------------------------------------------------
OptionsStorage::OptionsStorage(IConfigSchemePtr scheme) :
    m_scheme(scheme)
{
}

//-------------------------------------------------------------------------------------------------
OptionsStorage::~OptionsStorage()
{
}

//-------------------------------------------------------------------------------------------------
void OptionsStorage::setValue(const std::string &name, const OptionValueType &value)
{
    const Option newOption(name, value);
    m_scheme->checkOption(newOption);
    m_options[name] = newOption;
}

//-------------------------------------------------------------------------------------------------
const OptionValueType &OptionsStorage::getValue(const std::string &name) const
{
    Options::const_iterator i = m_options.find(name);
    if (i == m_options.end())
    {
        throw OptionNotFound("Option " + name + " not found in storage.", name);
    }

    return i->second.value();
}

//-------------------------------------------------------------------------------------------------
IConfigSchemePtr OptionsStorage::getScheme() const
{
    return m_scheme;
}

//-------------------------------------------------------------------------------------------------
OptionsStorage::CollectionType::Element *OptionsStorage::begin() const
{
    return new OptionsStorageElement(this, m_options.cbegin());
}

//-------------------------------------------------------------------------------------------------
OptionsStorage::CollectionType::Element *OptionsStorage::end() const
{
    return new OptionsStorageElement(this, m_options.cend());
}

//-------------------------------------------------------------------------------------------------
OptionsStorage::CollectionType::Element *OptionsStorage::next(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const OptionsStorageElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const OptionsStorageElement*>(current)->getIterator() != m_options.end());

    const OptionsStorageElement *element = dynamic_cast<const OptionsStorageElement*>(current);
    Options::const_iterator iterator = element->getIterator();

    return new OptionsStorageElement(this, ++iterator);
}

//-------------------------------------------------------------------------------------------------
const OptionsStorage::CollectionType::CollectionValueType &OptionsStorage::dereference(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const OptionsStorageElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const OptionsStorageElement*>(current)->getIterator() != m_options.end());

    return dynamic_cast<const OptionsStorageElement*>(current)->getIterator()->second;
}
