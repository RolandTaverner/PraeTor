#pragma once

#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

class IProcess
{
public:
    virtual ~IProcess() {}

    virtual const std::string &processType() const = 0;

    virtual const std::string &executable() const = 0;

    virtual const boost::filesystem::path &root() const = 0;

};

typedef boost::shared_ptr<IProcess> IProcessPtr;