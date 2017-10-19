#include <boost/assert.hpp>

#include "Options/OptionErrors.h"
#include "Options/OptionsStorage.h"

//-------------------------------------------------------------------------------------------------
OptionsStorage::OptionsStorage(IConfigSchemePtr scheme, bool addDefaultValues) :
    m_scheme(scheme)
{
    if (addDefaultValues)
    {
        for (const OptionDesc &desc : m_scheme->getRange())
        {
            if (desc.get<1>().is_initialized())
            {
                setValue(desc.get<0>(), desc.get<1>());
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
OptionsStorage::~OptionsStorage()
{
}

//-------------------------------------------------------------------------------------------------
void OptionsStorage::setValue(const std::string &name, const OptionValueType &value)
{
    if (value.is_initialized())
    {
        const Option newOption(name, value);
        m_scheme->checkOption(newOption);
        m_options[name] = newOption;
    }
    else
    {
        m_options.erase(name);
    }
}

//-------------------------------------------------------------------------------------------------
const OptionValueType &OptionsStorage::getValue(const std::string &name) const
{
    Options::const_iterator i = m_options.find(name);
    if (i == m_options.end())
    {
        throw OptionError(makeErrorCode(OptionErrors::notFoundInStorage), name);
    }

    return i->second.value();
}

//-------------------------------------------------------------------------------------------------
bool OptionsStorage::hasValue(const std::string &name) const
{
    return m_options.find(name) != m_options.end();
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

//-------------------------------------------------------------------------------------------------
std::string OptionsStorage::formatOption(const std::string &name, ISubstitutorPtr substitutorPtr) const
{
    Options::const_iterator i = m_options.find(name);
    if (i == m_options.end())
    {
        throw OptionError(makeErrorCode(OptionErrors::notFoundInStorage), name);
    }

    return getScheme()->formatOption(name, i->second.value().get(), substitutorPtr);
}