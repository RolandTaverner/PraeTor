#pragma once

#include <stdexcept>
#include <string>
#include <system_error>

#include "BaseError.h"

enum class ProcessErrors
{
	alreadyRunning = 1,
    noSuchStorage = 2,
    noSuchOption = 3,
    configFileWriteError = 4
};

namespace std
{
	template<> struct is_error_condition_enum<ProcessErrors>
	{
		static const bool value = true;
	};
} // namespace std

namespace Detail
{
	class ProcessErrorsCategory : public std::error_category
	{
	public:
		const char *name() const _NOEXCEPT override;
		std::string message(int value) const override;
	};
} // namespace Detail


class ProcessError
	: public BaseError
{	// base of all system-error exceptions
private:
	typedef BaseError _Mybase;

public:
	ProcessError(const std::error_condition &error)
		: _Mybase(error, "")
	{	// construct from error condition
	}

	ProcessError(const std::error_condition &error, const std::string &message)
		: _Mybase(error, message)
	{	// construct from error condition and message string
	}

	ProcessError(const std::error_condition &error, const char *message)
		: _Mybase(error, message)
	{	// construct from error condition and message string
	}

	ProcessError(int errVal, const std::error_category &errCat)
		: _Mybase(std::error_condition(errVal, errCat), "")
	{	// construct from error condition components
	}

	ProcessError(int errVal, const std::error_category &errCat, const std::string &message)
		: _Mybase(std::error_condition(errVal, errCat), message)
	{	// construct from error condition components and message string
	}

	ProcessError(int errVal, const std::error_category &errCat, const char *message)
		: _Mybase(std::error_condition(errVal, errCat), message)
	{	// construct from error condition components and message string
	}

	const std::error_condition &error() const _NOEXCEPT
	{	// return stored error condition
		return m_error;
	}
};

const std::error_category &getProcessErrorsCategory();

inline std::error_condition makeErrorCondition(ProcessErrors e)
{
	return std::error_condition(static_cast<int>(e), getProcessErrorsCategory());
}

