#include "Process/ProcessErrors.h"

namespace Detail
{
	const char *ProcessErrorsCategory::name() const _NOEXCEPT
	{
		return "ProcessErrors";
	}

	std::string ProcessErrorsCategory::message(int value) const
	{
		switch (value)
		{
		case static_cast<int>(ProcessErrors::alreadyRunning) :
			return "Process is already running.";
        case static_cast<int>(ProcessErrors::noSuchStorage) :
            return "Process has no specified storage.";
        case static_cast<int>(ProcessErrors::noSuchOption) :
            return "Process has no specified option.";
        case static_cast<int>(ProcessErrors::missingRequiredOption) :
            return "Option marked as required but no value provided.";
        case static_cast<int>(ProcessErrors::configFileWriteError) :
            return "Can't write config file.";
		}
		return "Process error";
	}

} /* namespace Detail */

  //--------------------------------------------------------------------------------------------------
const std::error_category &getProcessErrorsCategory()
{
	static const Detail::ProcessErrorsCategory instance;
	return instance;
}
