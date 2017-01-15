#pragma once

#include <map>
#include <stdexcept>
#include <string>

#include <boost/range/irange.hpp>
#include <boost/tokenizer.hpp>

#include <json/value.h>

typedef std::map<std::string, std::string> ResourceParameters;

class ResourceParser
{
public:
    explicit ResourceParser(const std::string &scheme);
    virtual ~ResourceParser();

    const Json::Value &scheme() const;

    std::string mapToAction(const std::string &resource, ResourceParameters &parameters) const;

private:
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    typedef boost::iterator_range<tokenizer::const_iterator> cirange;

    std::string mapToAction(const Json::Value &schemeNode, const cirange &tokens, ResourceParameters &parameters) const;

    Json::Value m_scheme;
};


class ResourceParserError : public std::runtime_error
{
public:
    ResourceParserError(const std::string &message):
        std::runtime_error(message)
    {
    }

    virtual ~ResourceParserError() {}
};

class NoActionError : public ResourceParserError
{
public:
    NoActionError(const std::string &message) :
        ResourceParserError(message)
    {
    }

    virtual ~NoActionError() {}
};

class CantMapResourceToActionError : public ResourceParserError
{
public:
    CantMapResourceToActionError(const std::string &message) :
        ResourceParserError(message)
    {
    }

    virtual ~CantMapResourceToActionError() {}
};
