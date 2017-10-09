#pragma once

#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>

#include "Tools/Configuration/ConfigurationView.h"

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

class IOptionConstraint
{
public:
    virtual ~IOptionConstraint() {}

    virtual bool isValid(const Option &opt) const = 0;

    virtual std::string description() const = 0;
};

typedef boost::shared_ptr<IOptionConstraint> IOptionConstraintPtr;
typedef std::list<IOptionConstraintPtr> OptionConstraints;

enum OptionValueTag { OVT_NUMBER, OVT_STRING, OVT_DOMAIN };

typedef std::set<std::string> OptionValueDomain;

//struct OptionDesc
//{
//    std::string name;
//    OptionValueType defaultValue;
//    bool isRequired;
//    OptionConstraints constraints;
//    bool isList;
//    OptionValueTag type;
//    bool isSystem;
//};

// 0 = name, 1 = defaultValue, 2 = isRequired, 3 = contraints, 4 = isList, 5 = type, 6 = isSystem, 7 - domain, 8 - format

typedef boost::tuple<std::string,
    OptionValueType,
    bool,
    OptionConstraints,
    bool,
    OptionValueTag,
    bool,
    OptionValueDomain,
    Tools::Configuration::ConfigurationView> OptionDesc;

typedef boost::tuple<OptionDesc, OptionValueType, std::string> OptionDescValue;


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