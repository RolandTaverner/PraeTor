#pragma once

#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Options/AbstractCollection.h"
#include "Options/IFormatter.h"
#include "Options/ISubstitutor.h"
#include "Options/Option.h"
#include "Options/OptionErrors.h"

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

    virtual std::string formatOption(const std::string &name, const OptionValueContainer &value, ISubstitutorPtr substitutorPtr) const = 0; /* throws */

    virtual void setFormatter(const std::string &name, IFormatterPtr formatterPtr) = 0; /* throws */
};

typedef boost::shared_ptr<IConfigScheme> IConfigSchemePtr;

struct OptionIsRequiredPred
{
    bool operator()(const OptionDesc &od) const { return od.get<2>(); }
};

struct OptionIsSystemPred
{
    bool operator()(const OptionDesc &od) const { return od.get<6>(); }
};