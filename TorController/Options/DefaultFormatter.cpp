#include <boost/algorithm/string/replace.hpp>

#include "DefaultFormatter.h"

//-------------------------------------------------------------------------------------------------
DefaultFormatter::DefaultFormatter()
{
}

//-------------------------------------------------------------------------------------------------
DefaultFormatter::~DefaultFormatter()
{
}

//-------------------------------------------------------------------------------------------------
void DefaultFormatter::format(const Option &option, 
                              const OptionDesc &desc,
                              std::ostream &out) const
{
    std::string format = desc.get<8>().get("");
    boost::algorithm::replace_all(format, "%NAME%", option.name());
    out << format;
}