#pragma once

#include <string>

#include <boost/shared_ptr.hpp>

class ISubstitutor
{
public:
    virtual ~ISubstitutor() {}

    virtual bool hasSubstitute(const std::string &value) const = 0;
    virtual std::string substituteValue(const std::string &value) const = 0;
};

typedef boost::shared_ptr<const ISubstitutor> ISubstitutorPtr;