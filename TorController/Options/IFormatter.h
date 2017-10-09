#pragma once

#include <iosfwd>

#include <boost/shared_ptr.hpp>

#include "Options/Option.h"

class IFormatter
{
public:
    virtual ~IFormatter() {}

    virtual void format(const Option &option, const OptionDesc &desc, std::ostream &out) const = 0;
};

typedef boost::shared_ptr<IFormatter> IFormatterPtr;