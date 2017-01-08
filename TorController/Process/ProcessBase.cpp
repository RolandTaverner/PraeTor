#include "Process/ProcessBase.h"

//-------------------------------------------------------------------------------------------------
ProcessBase::ProcessBase()
{
}

//-------------------------------------------------------------------------------------------------
ProcessBase::~ProcessBase()
{
}

//-------------------------------------------------------------------------------------------------
const std::string &ProcessBase::processType() const
{
    return m_processType;
}

//-------------------------------------------------------------------------------------------------
const std::string &ProcessBase::executable() const
{
    return m_executable;
}

//-------------------------------------------------------------------------------------------------
const boost::filesystem::path &ProcessBase::root() const
{
    return m_rootPath;
}
