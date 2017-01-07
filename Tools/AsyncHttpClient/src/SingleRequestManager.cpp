// Boost
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include "Tools/Logger/Logger.h"

// This
#include "Tools/AsyncHttpClient/SingleRequestManager.h"

namespace Tools
{
namespace AsyncHttpClient
{

//--------------------------------------------------------------------------------------------------
SingleRequestManager::SingleRequestManager(const SingleRequestHandler &handler,
                                           const IConnectionPoolPtr &connPoolPtr,
                                           const boost::shared_ptr<pion::scheduler> &schedulerPtr) :
        RequestManagerBase(connPoolPtr, schedulerPtr), m_handler(handler)
{
}

//--------------------------------------------------------------------------------------------------
SingleRequestManager::~SingleRequestManager()
{
}

//--------------------------------------------------------------------------------------------------
void SingleRequestManager::setRequest(pion::http::request_ptr request,
                                      const boost::asio::ip::tcp::endpoint &endpoint,
                                      bool keepAlive)
{
    if (getRequestsCount())
    {
        throw std::runtime_error("Request already set.");
    }

    addRequest(request, endpoint, keepAlive);
}

//--------------------------------------------------------------------------------------------------
void SingleRequestManager::onFinished(const RequestResponse &results)
{
    BOOST_ASSERT(results.size() == 1u);

    const RequestResponse::value_type &v = *results.begin();

    try
    {
        m_handler(RequestResult(v.first->getEndpoint(),
                                v.first->getRequest(),
                                v.second.first,
                                v.second.second));
    }
    catch (const std::exception &e)
    {
        // TODO: detailed message
        Tools::Logger::Logger::getInstance().error(std::string("RequestHandler() exception: ") + e.what());
    }
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */
