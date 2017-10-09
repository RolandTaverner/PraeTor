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
