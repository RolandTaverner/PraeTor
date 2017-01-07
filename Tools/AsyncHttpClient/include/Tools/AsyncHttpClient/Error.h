#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ERROR_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ERROR_H_

#include <boost/system/error_code.hpp>

namespace Tools
{
namespace AsyncHttpClient
{
namespace Error
{

enum MiscErrors
{
    internalError = 1,
    requestCanceled,
    requestTimedOut
};

namespace Detail
{

class MiscCategory : public boost::system::error_category
{
public:
    const char *name() const BOOST_SYSTEM_NOEXCEPT; // Название категории (AsyncHttpClient.Misc)
    std::string message(int value) const;
};

} /* namespace Detail */

const boost::system::error_category &getMiscCategory();

static const boost::system::error_category &miscCategory = getMiscCategory();

} /* namespace Error */
} /* namespace AsyncHttpClient */
} /* namespace Tools */

namespace boost
{
namespace system
{

template<> struct is_error_code_enum<Tools::AsyncHttpClient::Error::MiscErrors>
{
    static const bool value = true;
};

} /* namespace system */
} /* namespace boost */

namespace Tools
{
namespace AsyncHttpClient
{
namespace Error
{

inline boost::system::error_code makeErrorCode(MiscErrors e)
{
    return boost::system::error_code(static_cast<int>(e), getMiscCategory());
}

} /* namespace Error */
} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ERROR_H_ */
