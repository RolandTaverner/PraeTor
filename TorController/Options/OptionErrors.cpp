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
            return "More than one default value specified for single value option.";
        case static_cast<int>(OptionErrors::missingRequiredAttrInDefinition) :
            return "Required attribute not found.";
        case static_cast<int>(OptionErrors::unknownAttributeValueInDefinition) :
            return "Attribute value not in list.";
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
