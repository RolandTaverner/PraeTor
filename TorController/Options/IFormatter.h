#pragma once

#include <iosfwd>

#include "Options/Option.h"

class IFormatter
{
public:
    virtual ~IFormatter() {}

    virtual void format(const Option &option, std::ostream &out) = 0;
};