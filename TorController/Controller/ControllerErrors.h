#pragma once

#include <stdexcept>
#include <string>
#include <system_error>

#include "BaseError.h"

enum class ControllerErrors
{
	processNotFound = 1,
	startProcessError = 2
};

namespace std
{
	template<> struct is_error_condition_enum<ControllerErrors>
	{
		static const bool value = true;
	};
} // namespace std

namespace Detail
{
	class ControllerErrorsCategory : public std::error_category
	{
	public:
		const char *name() const _NOEXCEPT override;
		std::string message(int value) const override;
	};
} // namespace Detail


class ControllerError
	: public BaseError
{	// base of all system-error exceptions
private:
	typedef BaseError _Mybase;

public:
	ControllerError(const std::error_condition &error)
		: _Mybase(error, "")
	{	// construct from error condition
	}

	ControllerError(const std::error_condition &error, const std::string &message)
		: _Mybase(error, message)
	{	// construct from error condition and message string
	}

	ControllerError(const std::error_condition &error, const char *message)
		: _Mybase(error, message)
	{	// construct from error condition and message string
	}

	ControllerError(int errVal, const std::error_category &errCat)
		: _Mybase(std::error_condition(errVal, errCat), "")
	{	// construct from error condition components
	}

	ControllerError(int errVal, const std::error_category &errCat, const std::string &message)
		: _Mybase(std::error_condition(errVal, errCat), message)
	{	// construct from error condition components and message string
	}

	ControllerError(int errVal, const std::error_category &errCat, const char *message)
		: _Mybase(std::error_condition(errVal, errCat), message)
	{	// construct from error condition components and message string
	}

	const std::error_condition &error() const _NOEXCEPT
	{	// return stored error condition
		return m_error;
	}
};

const std::error_category &getControllerErrorsCategory();

inline std::error_condition makeErrorCondition(ControllerErrors e)
{
	return std::error_condition(static_cast<int>(e), getControllerErrorsCategory());
}

