#pragma once

#include <system_error>

enum class ControllerErrors
{
	unknownError = 1,
    processNotFound = 2,
	startProcessError = 3,
    presetsNotFound = 4
};

namespace std
{
	template<> struct is_error_code_enum<ControllerErrors>
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


const std::error_category &getControllerErrorsCategory();

inline std::error_code makeErrorCode(ControllerErrors e)
{
	return std::error_code(static_cast<int>(e), getControllerErrorsCategory());
}

struct ControllerError : std::system_error
{
    using std::system_error::system_error;
};
