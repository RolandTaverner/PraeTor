#include "Options/OptionErrors.h"

namespace Detail
{
    const char *OptionErrorsCategory::name() const _NOEXCEPT
    {
        return "OptionErrors";
    }

    std::string OptionErrorsCategory::message(int value) const
    {
        switch (value)
        {
        case static_cast<int>(OptionErrors::optionError) :
            return "Generic option error.";
        case static_cast<int>(OptionErrors::notRegistered) :
            return "Option is not registered.";
        case static_cast<int>(OptionErrors::alreadyRegistered) :
            return "Option already registered";
        case static_cast<int>(OptionErrors::constraintCheckFailed) :
            return "Constraints check failed.";
        case static_cast<int>(OptionErrors::invalidDefinition) :
            return "Generic option error.";
        case static_cast<int>(OptionErrors::missingDefaultValue) :
            return "Option has no default value.";
        case static_cast<int>(OptionErrors::missingDomain) :
            return "Option has no domain.";
        case static_cast<int>(OptionErrors::notFoundInStorage) :
            return "Option not found in storage.";
        case static_cast<int>(OptionErrors::emptyDomain) :
            return "Empty domain not allowed.";
        case static_cast<int>(OptionErrors::assigningListToSingleValue) :
            return "Can't assign list value to single value option.";
        case static_cast<int>(OptionErrors::assigningSingleToListValue) :
            return "Can't assign single value to list value option.";
        case static_cast<int>(OptionErrors::missingRequiredAttrInDefinition) :
            return "Required attribute not found.";
        case static_cast<int>(OptionErrors::unknownAttributeValueInDefinition) :
            return "Attribute value not in list.";
        case static_cast<int>(OptionErrors::missingValue) :
            return "Can't assign empty value to required option.";
        case static_cast<int>(OptionErrors::typeCheckFailed) :
            return "Option value does not conform it's type.";
        }
        return "Option error";
    }

} /* namespace Detail */

  //--------------------------------------------------------------------------------------------------
const std::error_category &getOptionErrorsCategory()
{
    static const Detail::OptionErrorsCategory instance;
    return instance;
}
