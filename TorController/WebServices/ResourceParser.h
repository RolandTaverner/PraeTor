#pragma once

#include <map>
#include <string>

#include <json/value.h>

typedef std::map<std::string, std::string> ResourceParameters;

class ResourceParser
{
public:
    explicit ResourceParser(const std::string &scheme);
    virtual ~ResourceParser();

    const Json::Value &scheme() const;

    bool mapToAction(const std::string &resource, std::string &action, ResourceParameters &parameters) const;

private:
    Json::Value m_scheme;
};

