#pragma once

#include <map>
#include <stdexcept>

#include "Options/IOptionsStorage.h"

enum OptionsStorageType { OST_CMDLINEARGS, OST_CONFIGFILE, OST_RUNTIME };

class ProcessConfiguration
{
public:
    ProcessConfiguration(IOptionsStoragePtr cmdOpts, IOptionsStoragePtr confOpts, IOptionsStoragePtr rtOpts);
    virtual ~ProcessConfiguration();

    bool hasStorage(const OptionsStorageType storageType) const;

    IOptionsStoragePtr getStorage(const OptionsStorageType storageType);

    IOptionsStorageConstPtr getStorage(const OptionsStorageType storageType) const;

private:
    typedef std::map<OptionsStorageType, IOptionsStoragePtr> OptionsStorages;

    OptionsStorages m_storages;
};

