#pragma once

#include <stdexcept>
#include <string>

#include <boost/optional.hpp>

typedef boost::optional<std::string> OptionValueType;

class Option
{
public:
    Option();
    Option(const std::string &name, const OptionValueType &value);
    virtual ~Option();

    const std::string &name() const;
    const OptionValueType &value() const;
    void setValue(const OptionValueType &newValue);

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