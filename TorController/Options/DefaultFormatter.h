#pragma once
#include "IFormatter.h"

class DefaultFormatter :
    public IFormatter
{
public:
    DefaultFormatter();
    ~DefaultFormatter();
    
    void format(const Option &option, 
                const OptionDesc &desc,
                std::ostream &out) const override;
};

