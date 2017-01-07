#include "Process/ProcessBase.h"

ProcessBase::ProcessBase()
{
}

ProcessBase::~ProcessBase()
{
}

const std::string &ProcessBase::processType() const
{
    return m_processType;
}