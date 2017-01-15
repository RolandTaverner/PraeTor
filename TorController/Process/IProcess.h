#pragma once

#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include "Process/ProcessConfiguration.h"

class IProcess
{
public:
    virtual ~IProcess() {}

    virtual const std::string &name() const = 0;

    virtual const std::string &executable() const = 0;

    virtual const boost::filesystem::path &rootPath() const = 0;

    virtual const ProcessConfiguration &getConfiguration() const = 0;
};

typedef boost::shared_ptr<IProcess> IProcessPtr;