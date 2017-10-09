#pragma once

#include <stdexcept>
#include <string>
#include <system_error>

class BaseError
	: public std::runtime_error
{	// base of all controller-error exceptions
private:
	static std::string MakeStr(const std::error_condition &error, std::string message)
	{	// compose error message
		if (!message.empty())
		{
			message.append(": ");
		}

		message.append(error.message());
		return (message);
	}

protected:
	BaseError(const std::error_condition &error, const std::string &message)
		: runtime_error(MakeStr(error, message)),
		m_error(error)
	{	// construct from error condition and message string
	}

	std::error_condition m_error;	// the stored error condition
};