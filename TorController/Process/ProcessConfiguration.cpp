#include "ProcessConfiguration.h"

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::ProcessConfiguration(IOptionsStoragePtr cmdOpts, IOptionsStoragePtr confOpts, IOptionsStoragePtr rtOpts)
{
    if (cmdOpts)
    {
        m_storages[OST_CMDLINEARGS] = cmdOpts;
    }
    if (confOpts)
    {
        m_storages[OST_CONFIGFILE] = confOpts;
    }
    if (rtOpts)
    {
        m_storages[OST_RUNTIME] = rtOpts;
    }
}

//-------------------------------------------------------------------------------------------------
ProcessConfiguration::~ProcessConfiguration()
{
}

//-------------------------------------------------------------------------------------------------
bool ProcessConfiguration::hasStorage(const OptionsStorageType storageType) const
{
    return m_storages.find(storageType) != m_storages.end();
}

//-------------------------------------------------------------------------------------------------
IOptionsStoragePtr ProcessConfiguration::getStorage(const OptionsStorageType storageType)
{
    OptionsStorages::iterator i = m_storages.find(storageType);
    if (i == m_storages.end())
    {
        throw std::range_error("No such storage.");
    }
    return i->second;
}

//-------------------------------------------------------------------------------------------------
IOptionsStorageConstPtr ProcessConfiguration::getStorage(const OptionsStorageType storageType) const
{
    OptionsStorages::const_iterator i = m_storages.find(storageType);
    if (i == m_storages.end())
    {
        throw std::range_error("No such storage.");
    }
    return i->second;
}