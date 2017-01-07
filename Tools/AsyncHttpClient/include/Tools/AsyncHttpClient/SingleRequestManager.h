#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_SINGLEREQUESTMANAGER_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_SINGLEREQUESTMANAGER_H_

// This
#include "Tools/AsyncHttpClient/HandlerTypes.h"
#include "Tools/AsyncHttpClient/RequestManagerBase.h"

namespace Tools
{
namespace AsyncHttpClient
{

class SingleRequestManager : public RequestManagerBase
{
public:
    SingleRequestManager(const SingleRequestHandler &handler,
                         const IConnectionPoolPtr &connPoolPtr,
                         const boost::shared_ptr<pion::scheduler> &schedulerPtr);
    virtual ~SingleRequestManager();

    void setRequest(pion::http::request_ptr request,
                    const boost::asio::ip::tcp::endpoint &endpoint,
                    bool keepAlive);

protected:
    virtual void onFinished(const RequestResponse &results);

private:
    SingleRequestHandler m_handler;
};

typedef boost::shared_ptr<SingleRequestManager> SingleRequestManagerPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_SINGLEREQUESTMANAGER_H_ */
