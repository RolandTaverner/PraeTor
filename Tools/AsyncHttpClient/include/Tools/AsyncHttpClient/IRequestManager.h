#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUESTMANAGER_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUESTMANAGER_H_

// Boost
#include <boost/noncopyable.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/shared_ptr.hpp>

// Pion
#include <pion/http/request.hpp>

// This
#include "Tools/AsyncHttpClient/IRequest.h"

namespace Tools
{
namespace AsyncHttpClient
{

class IRequestManager : private ::boost::noncopyable
{
public:
    virtual ~IRequestManager() {}

    virtual void start(const boost::posix_time::time_duration &timeout) = 0;

    virtual void cancel() = 0;
};

typedef boost::shared_ptr<IRequestManager> IRequestManagerPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_IREQUESTMANAGER_H_ */
