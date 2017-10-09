#include "ProcessConfiguration.h"

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::ProcessConfiguration()
{
}

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::~ProcessConfiguration()
{
}

//-------------------------------------------------------------------------------------------------
bool ProcessConfiguration::hasStorage(const std::string &name) const
{
    return m_storages.find(name) != m_storages.end();
}

//-------------------------------------------------------------------------------------------------
IOptionsStoragePtr ProcessConfiguration::getStorage(const std::string &name)
{
    OptionsStorages::iterator i = m_storages.find(name);
    if (i == m_storages.end())
    {
        throw std::range_error("No such storage.");
    }
    return i->second;
}

//-------------------------------------------------------------------------------------------------
IOptionsStorageConstPtr ProcessConfiguration::getStorage(const std::string &name) const
{
    OptionsStorages::const_iterator i = m_storages.find(name);
    if (i == m_storages.end())
    {
        throw std::range_error("No such storage.");
    }
    return i->second;
}

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::CollectionType::Element *ProcessConfiguration::begin() const
{
    return new ProcessConfigElement(this, m_storages.cbegin());
}

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::CollectionType::Element *ProcessConfiguration::end() const
{
    return new ProcessConfigElement(this, m_storages.cend());
}

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::CollectionType::Element *ProcessConfiguration::next(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const ProcessConfigElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const ProcessConfigElement*>(current)->getIterator() != m_storages.end());

    const ProcessConfigElement *element = dynamic_cast<const ProcessConfigElement*>(current);
    OptionsStorages::const_iterator iterator = element->getIterator();

    return new ProcessConfigElement(this, ++iterator);
}

//-------------------------------------------------------------------------------------------------
const ProcessConfiguration::CollectionType::CollectionValueType &ProcessConfiguration::dereference(const CollectionType::Element *current) const
{
    BOOST_ASSERT(current != NULL);
    BOOST_ASSERT(dynamic_cast<const ProcessConfigElement*>(current) != NULL);
    BOOST_ASSERT(dynamic_cast<const ProcessConfigElement*>(current)->getIterator() != m_storages.end());

    return dynamic_cast<const ProcessConfigElement*>(current)->getIterator()->second;
}

//-------------------------------------------------------------------------------------------------
void ProcessConfiguration::addStorage(const std::string &name, IOptionsStoragePtr storagePtr)
{
    if (m_storages.find(name) != m_storages.end())
    {
        throw std::runtime_error("Storage " + name + " already registered.");
    }
    m_storages[name] = storagePtr;
}
