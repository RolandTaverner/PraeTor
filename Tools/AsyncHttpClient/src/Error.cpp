// This
#include "Tools/AsyncHttpClient/Error.h"

namespace Tools
{
namespace AsyncHttpClient
{
namespace Error
{
namespace Detail
{

//--------------------------------------------------------------------------------------------------
const char *MiscCategory::name() const BOOST_SYSTEM_NOEXCEPT
{
    return "Tools.AsyncHttpClient.Misc";
}

//--------------------------------------------------------------------------------------------------
std::string MiscCategory::message(int value) const
{
    switch (value)
    {
        case Error::internalError:
            return "Internal error";
        case Error::requestCanceled:
            return "Request canceled";
        case Error::requestTimedOut:
            return "Request timed out";
    }

    return "AsyncHttpClient.Misc error";
}

} /* namespace Detail */

//--------------------------------------------------------------------------------------------------
const boost::system::error_category &getMiscCategory()
{
    static Detail::MiscCategory instance;
    return instance;
}

} /* namespace Error */
} /* namespace AsyncHttpClient */
} /* namespace Tools */
