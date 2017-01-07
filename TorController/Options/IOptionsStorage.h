#pragma once

#include <stdexcept>
#include <string>

#include "Options/AbstractCollection.h"
#include "Options/Option.h"

class IOptionsStorage : public AbstractCollection<Option>
{
public:
    virtual ~IOptionsStorage() {}

    virtual void setValue(const std::string &name, const OptionValueType &value) = 0; /* throws */
    virtual const OptionValueType &getValue(const std::string &name) const = 0; /* throws */
};

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