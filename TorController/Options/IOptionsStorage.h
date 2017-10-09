#pragma once

#include <stdexcept>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Options/AbstractCollection.h"
#include "Options/IConfigScheme.h"
#include "Options/Option.h"

class IOptionsStorage : public AbstractCollection<Option>
{
public:
    virtual ~IOptionsStorage() {}

    virtual void setValue(const std::string &name, const OptionValueType &value) = 0; /* throws */
    virtual const OptionValueType &getValue(const std::string &name) const = 0; /* throws */
    virtual bool hasValue(const std::string &name) const = 0;
    virtual IConfigSchemePtr getScheme() const = 0;
};

typedef boost::shared_ptr<IOptionsStorage> IOptionsStoragePtr;
typedef boost::shared_ptr<const IOptionsStorage> IOptionsStorageConstPtr;

class OptionsStorageError : public OptionError
{
public:
    OptionsStorageError(const std::string &message, const std::string &name) :
        OptionError(message), m_name(name)
    {
    }

    virtual ~OptionsStorageError()
    {
    }

    const std::string &getName() const
    {
        return m_name;
    }

private:
    std::string m_name;
};

class OptionNotFound : public OptionsStorageError
{
public:
    OptionNotFound(const std::string &message, const std::string &name) :
        OptionsStorageError(message, name)
    {
    }

    virtual ~OptionNotFound()
    {
    }
};