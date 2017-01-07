#pragma once

#include <string>

#include <boost/filesystem/path.hpp>

#include "Options/IOptionsStorage.h"

class ProcessBase
{
public:
    ProcessBase();
    virtual ~ProcessBase();

    const std::string &processType() const;

private:
    std::string m_processType;
};

