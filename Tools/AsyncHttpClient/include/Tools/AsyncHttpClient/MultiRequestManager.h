#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_MULTIREQUESTMANAGER_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_MULTIREQUESTMANAGER_H_

// This
#include "Tools/AsyncHttpClient/HandlerTypes.h"
#include "Tools/AsyncHttpClient/RequestManagerBase.h"

namespace Tools
{
namespace AsyncHttpClient
{

class MultiRequestManager : public RequestManagerBase
{
public:
    MultiRequestManager(const MultiRequestHandler &handler,
                        const IConnectionPoolPtr &connPoolPtr,
                        const boost::shared_ptr<pion::scheduler> &schedulerPtr);
    virtual ~MultiRequestManager();

    using RequestManagerBase::addRequest;

protected:
    virtual void onFinished(const RequestResponse &results);

private:
    MultiRequestHandler m_handler;
};

typedef boost::shared_ptr<MultiRequestManager> MultiRequestManagerPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_MULTIREQUESTMANAGER_H_ */
