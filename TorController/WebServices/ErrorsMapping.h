#pragma once

#include <string>
#include "Error.h"

class ErrorsMapping
{
public:
    ErrorsMapping();
    ~ErrorsMapping();

    unsigned getHttpStatusCode(const std::string &action, const std::string &method, const ErrorCode &error) const;
};

