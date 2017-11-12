#include "Controller/ControllerErrors.h"
#include "Options/OptionErrors.h"
#include "Process/ProcessErrors.h"
#include "WebServices/ErrorsMapping.h"


//-------------------------------------------------------------------------------------------------
ErrorsMapping::ErrorsMapping()
{
}

//-------------------------------------------------------------------------------------------------
ErrorsMapping::~ErrorsMapping()
{
}

//-------------------------------------------------------------------------------------------------
unsigned ErrorsMapping::getHttpStatusCode(const std::string &action, const std::string &method, const ErrorCode &error) const
{
    return 500u;
}