#include <boost/algorithm/string.hpp>

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
std::string ResourceParser::mapToAction(const std::string &resource, ResourceParameters &parameters) const
{
    boost::char_separator<char> sep("/");
    tokenizer tokens(resource, sep);
    
    
    return mapToAction(scheme(), boost::make_iterator_range(tokens), parameters);
}

//-------------------------------------------------------------------------------------------------
std::string ResourceParser::mapToAction(const Json::Value &schemeNode, const cirange &tokens, ResourceParameters &parameters) const
{
    BOOST_ASSERT(!tokens.empty());

    const std::string node = schemeNode.get("node", Json::Value(Json::stringValue)).asString();
    
    if (!boost::starts_with(node, "$"))
    {
        if (!boost::equals(node, *tokens.begin()))
        {
            throw CantMapResourceToActionError("");
        }
    }
    else
    {
        const std::string parameterName = boost::erase_first_copy(node, "$");
        parameters[parameterName] = *tokens.begin();
    }

    cirange tokensTail(tokens);
    tokensTail.advance_begin(1);

    if (tokensTail.empty())
    {
        if (!schemeNode.isMember("action"))
        {
            throw NoActionError("No action at node " + node);
        }
        return schemeNode["action"].asString();
    }

    if (schemeNode.isMember("next")
        && schemeNode.get("next", Json::Value(Json::arrayValue)).isArray())
    {
        const Json::Value next = schemeNode.get("next", Json::Value(Json::arrayValue));

        const Json::Value::ArrayIndex count = next.size();
        for (Json::Value::ArrayIndex i = 0; i < count; ++i)
        {
            const Json::Value &nextSchemeNode = next[i];
            try
            {
                return mapToAction(nextSchemeNode, tokensTail, parameters);
            }
            catch (const CantMapResourceToActionError&)
            {
                continue;
            }
        }
    }

    throw CantMapResourceToActionError("Can't find " + *tokensTail.begin() + " in scheme.");
    return "";
}