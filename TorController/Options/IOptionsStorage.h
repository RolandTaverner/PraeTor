#pragma once

#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Options/AbstractCollection.h"
#include "Options/IConfigScheme.h"
#include "Options/ISubstitutor.h"
#include "Options/Option.h"
#include "Options/OptionErrors.h"

class IOptionsStorage : public AbstractCollection<Option>
{
public:
    virtual ~IOptionsStorage() {}

    virtual void setValue(const std::string &name, const OptionValueType &value) = 0; /* throws */
    virtual const OptionValueType &getValue(const std::string &name) const = 0; /* throws */
    virtual bool hasValue(const std::string &name) const = 0;
    virtual std::string formatOption(const std::string &name, ISubstitutorPtr substitutorPtr) const = 0;
    virtual IConfigSchemePtr getScheme() const = 0;
    //virtual bool validate() const = 0;
};

typedef boost::shared_ptr<IOptionsStorage> IOptionsStoragePtr;
typedef boost::shared_ptr<const IOptionsStorage> IOptionsStorageConstPtr;
