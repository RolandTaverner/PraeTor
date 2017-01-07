// Boost
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

// This
#include "Tools/AsyncHttpClient/AsyncHttpClient.h"
#include "Tools/AsyncHttpClient/MultiRequestManager.h"
#include "Tools/AsyncHttpClient/SingleRequestManager.h"

namespace Tools
{
namespace AsyncHttpClient
{

//--------------------------------------------------------------------------------------------------
AsyncHttpClient::AsyncHttpClient()
{
}

//--------------------------------------------------------------------------------------------------
AsyncHttpClient::AsyncHttpClient(IConnectionPoolPtr connPoolPtr,
                                 const boost::shared_ptr<pion::scheduler> &schedulerPtr) :
        m_connPoolPtr(connPoolPtr), m_schedulerPtr(schedulerPtr)
{
}

//--------------------------------------------------------------------------------------------------
AsyncHttpClient::~AsyncHttpClient()
{
}

//--------------------------------------------------------------------------------------------------
IRequestManagerPtr AsyncHttpClient::createRequest(const RequestDesc &request,
                                                  const SingleRequestHandler &finishedHandler,
                                                  const boost::posix_time::time_duration &timeout,
                                                  bool startImmediately)
{
    SingleRequestManagerPtr managerPtr(new SingleRequestManager(finishedHandler,
                                                                m_connPoolPtr,
                                                                m_schedulerPtr));

    managerPtr->setRequest(request.get<0>(), request.get<1>(), request.get<2>());

    if (startImmediately)
    {
        managerPtr->start(timeout);
    }

    return managerPtr;
}

//--------------------------------------------------------------------------------------------------
IRequestManagerPtr AsyncHttpClient::createRequest(const MultiRequestDesc &request,
                                                  const MultiRequestHandler &finishedHandler,
                                                  const boost::posix_time::time_duration &timeout,
                                                  bool startImmediately)
{
    MultiRequestManagerPtr managerPtr(new MultiRequestManager(finishedHandler,
                                                              m_connPoolPtr,
                                                              m_schedulerPtr));

    BOOST_FOREACH(const MultiRequestDesc::value_type &i, request)
    {
        managerPtr->addRequest(i.get<0>(), i.get<1>(), i.get<2>());
    }

    if (startImmediately)
    {
        managerPtr->start(timeout);
    }

    return managerPtr;
}

typedef boost::promise<RequestResult> SingleRequestPromise;
typedef boost::shared_ptr<SingleRequestPromise> SingleRequestPromisePtr;

void singleRequestPromiseHandler(SingleRequestPromisePtr promisePtr,
                                 const RequestResult &result)
{
    promisePtr->set_value(result);
}

//--------------------------------------------------------------------------------------------------
SingleRequestFuture AsyncHttpClient::createFuture(const RequestDesc &request,
                                                  const boost::posix_time::time_duration &timeout)
{
    SingleRequestPromisePtr promisePtr(new SingleRequestPromise());

    SingleRequestHandler handler(boost::bind<>(&singleRequestPromiseHandler,
                                               promisePtr,
                                               _1));

    SingleRequestManagerPtr managerPtr(new SingleRequestManager(handler,
                                                                m_connPoolPtr,
                                                                m_schedulerPtr));

    managerPtr->setRequest(request.get<0>(), request.get<1>(), request.get<2>());

    managerPtr->start(timeout);

    return promisePtr->get_future();
}

typedef boost::promise<RequestResults> MultiRequestPromise;
typedef boost::shared_ptr<MultiRequestPromise> MultiRequestPromisePtr;

void multiRequestPromiseHandler(MultiRequestPromisePtr promisePtr,
                                 const RequestResults &result)
{
    promisePtr->set_value(result);
}

//--------------------------------------------------------------------------------------------------
MultiRequestFuture AsyncHttpClient::createFuture(const MultiRequestDesc &request,
                                                 const boost::posix_time::time_duration &timeout)
{
    MultiRequestPromisePtr promisePtr(new MultiRequestPromise());

    MultiRequestHandler handler(boost::bind<>(&multiRequestPromiseHandler,
                                              promisePtr,
                                              _1));

    MultiRequestManagerPtr managerPtr(new MultiRequestManager(handler,
                                                              m_connPoolPtr,
                                                              m_schedulerPtr));

    BOOST_FOREACH(const MultiRequestDesc::value_type &i, request)
    {
        managerPtr->addRequest(i.get<0>(), i.get<1>(), i.get<2>());
    }

    managerPtr->start(timeout);

    return promisePtr->get_future();
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */
