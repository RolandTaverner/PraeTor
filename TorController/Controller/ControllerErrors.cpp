#include "Controller/ControllerErrors.h"

namespace Detail
{
	const char *ControllerErrorsCategory::name() const _NOEXCEPT
	{
		return "ControllerErrors";
	}

	std::string ControllerErrorsCategory::message(int value) const
	{
		switch (value)
		{
		case static_cast<int>(ControllerErrors::processNotFound):
			return "Process not found."; 
		case static_cast<int>(ControllerErrors::startProcessError) :
			return "Start process error.";
        case static_cast<int>(ControllerErrors::presetsNotFound) :
            return "Presets group not found.";
        case static_cast<int>(ControllerErrors::processIsRunning) :
            return "Process is running.";
        }
		return "Controller error";
	}

} /* namespace Detail */

//--------------------------------------------------------------------------------------------------
const std::error_category &getControllerErrorsCategory()
{
	static const Detail::ControllerErrorsCategory instance;
	return instance;
}
