#include <json/reader.h>

#include "WebServices/ResourceParser.h"

//-------------------------------------------------------------------------------------------------
ResourceParser::ResourceParser(const std::string &scheme)
{
    if (!Json::Reader().parse(scheme, m_scheme, false))
    {
        throw std::runtime_error("Invalid resource scheme.");
    }
}

//-------------------------------------------------------------------------------------------------
ResourceParser::~ResourceParser()
{
}

//-------------------------------------------------------------------------------------------------
const Json::Value &ResourceParser::scheme() const
{
    return m_scheme;
}

//-------------------------------------------------------------------------------------------------
bool ResourceParser::mapToAction(const std::string &resource, std::string &action, ResourceParameters &parameters) const
{

    return false;
}