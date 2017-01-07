#include "Options/Option.h"

Option::Option()
{
}

Option::Option(const std::string &name, const OptionValueType &value) :
    m_name(name), m_value(value)
{
}

Option::~Option()
{
}

const std::string &Option::name() const
{
    return m_name;
}

const OptionValueType &Option::value() const
{
    return m_value;
}

void Option::setValue(const OptionValueType &newValue)
{ 
    m_value = newValue;
}