#pragma once

#include <system_error>

enum class OptionErrors
{
    optionError = 1,
    notRegistered = 2,
    alreadyRegistered = 3,
    constraintCheckFailed = 4,
    invalidDefinition = 5,
    missingDefaultValue = 6,
    missingDomain = 7,
    notFoundInStorage = 8,
    emptyDomain = 9,
    assigningListToSingleValue = 10,
    missingRequiredAttrInDefinition = 11,
    unknownAttributeValueInDefinition = 12
};

namespace std
{
    template<> struct is_error_code_enum<OptionErrors>
    {
        static const bool value = true;
    };
} // namespace std

namespace Detail
{
    class OptionErrorsCategory : public std::error_category
    {
    public:
        const char *name() const _NOEXCEPT override;
        std::string message(int value) const override;
    };
} // namespace Detail


const std::error_category &getOptionErrorsCategory();

inline std::error_code makeErrorCode(OptionErrors e)
{
    return std::error_code(static_cast<int>(e), getOptionErrorsCategory());
}

struct OptionError : std::system_error
{
    using std::system_error::system_error;
};
