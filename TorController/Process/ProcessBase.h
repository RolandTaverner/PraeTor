#pragma once

#include <string>

#include "Options/IOptionsStorage.h"
#include "Process/IProcess.h"

class ProcessBase : public IProcess
{
public:
    ProcessBase();
    virtual ~ProcessBase();

    // IProcess
    const std::string &processType() const override;

    const std::string &executable() const override;

    const boost::filesystem::path &root() const override;

private:
    std::string m_processType;
    std::string m_executable;
    boost::filesystem::path m_rootPath;
};

