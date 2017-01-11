#pragma once

#include <list>
#include <stdexcept>
#include <string>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

typedef std::string OptionSingleValue;
typedef std::list<OptionSingleValue> OptionListValue;
typedef boost::variant<OptionSingleValue, OptionListValue> OptionValueContainer;
typedef boost::optional<OptionValueContainer> OptionValueType;

class Option
{
public:
    Option();
    Option(const std::string &name, const OptionValueType &value);
    virtual ~Option();

    const std::string &name() const;
    const OptionValueType &value() const;
    void setValue(const OptionValueType &newValue);
    bool hasValue() const;

private:
    std::string m_name;
    OptionValueType m_value;
};

class OptionError : public std::runtime_error
{
public:
    explicit OptionError(const std::string &message) :
        std::runtime_error(message)
    {
    }

    virtual ~OptionError()
    {
    }
};