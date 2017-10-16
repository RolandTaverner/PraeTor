#pragma once

#include <system_error>

enum class ProcessErrors
{
	alreadyRunning = 1,
    noSuchStorage = 2,
    noSuchOption = 3,
    missingRequiredOption = 4,
    substitutionNotFound = 5,
    configFileWriteError = 6,
    processNotRunning = 7,
    systemOptionEditForbidden = 8,
    cantEditConfigOfRunningProcess = 9
};

namespace std
{
	template<> struct is_error_code_enum<ProcessErrors>
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


const std::error_category &getProcessErrorsCategory();

inline std::error_code makeErrorCode(ProcessErrors e)
{
	return std::error_code(static_cast<int>(e), getProcessErrorsCategory());
}

struct ProcessError : std::system_error
{
    using std::system_error::system_error;
};
