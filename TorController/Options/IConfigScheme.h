#pragma once

#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Options/AbstractCollection.h"
#include "Options/IFormatter.h"
#include "Options/Option.h"

class IConfigScheme : public AbstractCollection<OptionDesc>
{
public:
    virtual ~IConfigScheme() {}

    virtual bool hasOption(const std::string &name) const = 0;

    virtual void checkOption(const Option &opt) = 0; /* throws */

    virtual void registerOption(const std::string &name,
        const OptionValueType &defaultValue,
        bool required,
        const OptionConstraints &constraints,
        bool isList,
        OptionValueTag tag,
        bool isSystem,
        const OptionValueDomain &domain,
        const Tools::Configuration::ConfigurationView &format) = 0;  /* throws */

    virtual bool isRequired(const std::string &name) const = 0; /* throws */

    virtual bool hasDefaultValue(const std::string &name) const = 0; /* throws */

    virtual bool isList(const std::string &name) const = 0; /* throws */

    virtual bool isSystem(const std::string &name) const = 0; /* throws */

    virtual OptionValueContainer getDefaultValue(const std::string &name) const = 0; /* throws */

    virtual const OptionDesc &getOptionDesc(const std::string &name) const = 0; /* throws */

    virtual const OptionValueDomain &getOptionValueDomain(const std::string &name) const = 0; /* throws */

    virtual std::string formatOption(const std::string &name, const OptionValueContainer &value) const = 0; /* throws */

    virtual void setFormatter(const std::string &name, IFormatterPtr formatterPtr) = 0; /* throws */
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


class OptionDefinitionError : public OptionsSchemeError
{
public:
    OptionDefinitionError(const std::string &message, const std::string &name) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~OptionDefinitionError()
    {
    }
};

class OptionHasNoDefaultValue : public OptionsSchemeError
{
public:
    OptionHasNoDefaultValue(const std::string &message, const std::string &name) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~OptionHasNoDefaultValue()
    {
    }
};

class OptionValueHasNoDomain : public OptionsSchemeError
{
public:
    OptionValueHasNoDomain(const std::string &message, const std::string &name) :
        OptionsSchemeError(message, name)
    {
    }

    virtual ~OptionValueHasNoDomain()
    {
    }
};