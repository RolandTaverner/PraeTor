#pragma once

#include <list>
#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include "Options/AbstractCollection.h"
#include "Options/Option.h"

class IOptionConstraint
{
public:
    virtual ~IOptionConstraint() {}

    virtual bool isValid(const Option &opt) = 0;

    virtual std::string description() const = 0;
};

typedef boost::shared_ptr<IOptionConstraint> IOptionConstraintPtr;
typedef std::list<IOptionConstraintPtr> OptionConstraints;

typedef boost::tuple<std::string, OptionValueType, bool, OptionConstraints> OptionDesc;

class IConfigScheme : public AbstractCollection<OptionDesc>
{
public:
    virtual ~IConfigScheme() {}

    virtual void checkOption(const Option &opt) = 0; /* throws */

    virtual void registerOption(const std::string &name,
        const OptionValueType &defaultValue,
        bool required,
        const OptionConstraints &constraints) = 0;  /* throws */

    virtual bool isRequired(const std::string &name) const = 0; /* throws */

    virtual bool hasDefaultValue(const std::string &name) const = 0; /* throws */

    virtual const OptionDesc &getOptionDesc(const std::string &name) const = 0; /* throws */
};

typedef boost::shared_ptr<IConfigScheme> IConfigSchemePtr;

class OptionsSchemeError : public OptionError
{
public:
    OptionsSchemeError(const std::string &message, const std::string &name) :
        OptionError(message), m_name(name)
    {
    }

    virtual ~OptionsSchemeError()
    {
    }

    const std::string &getName() const
    {
        return m_name;
    }

private:
    std::string m_name;
};

class OptionNotRegistered : public OptionsSchemeError
{
public:
    OptionNotRegistered(const std::string &message, const std::string &name) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~OptionNotRegistered()
    {
    }
};

class OptionAlreadyRegistered : public OptionsSchemeError
{
public:
    OptionAlreadyRegistered(const std::string &message, const std::string &name) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~OptionAlreadyRegistered()
    {
    }
};

class ConstraintCheckFailed : public OptionsSchemeError
{
public:
    ConstraintCheckFailed(const std::string &message, const std::string &name, const OptionConstraints &failedConstraints) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~ConstraintCheckFailed()
    {
    }

    const OptionConstraints &failedCOnstraints()
    {
        return m_failedConstraints;
    }

private:
    OptionConstraints m_failedConstraints;
};
