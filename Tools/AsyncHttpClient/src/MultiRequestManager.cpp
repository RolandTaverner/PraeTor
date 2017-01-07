// C++
#include <string>

// Boost
#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "Tools/Logger/Logger.h"

// This
#include "Tools/AsyncHttpClient/MultiRequestManager.h"

namespace Tools
{
namespace AsyncHttpClient
{

//--------------------------------------------------------------------------------------------------
MultiRequestManager::MultiRequestManager(const MultiRequestHandler &handler,
                                         const IConnectionPoolPtr &connPoolPtr,
                                         const boost::shared_ptr<pion::scheduler> &schedulerPtr) :
        RequestManagerBase(connPoolPtr, schedulerPtr), m_handler(handler)
{
}

//--------------------------------------------------------------------------------------------------
MultiRequestManager::~MultiRequestManager()
{
}

//--------------------------------------------------------------------------------------------------
void MultiRequestManager::onFinished(const RequestResponse &results)
{
    BOOST_ASSERT(results.size() >= 1u);

    RequestResults requestResults;

    BOOST_FOREACH(const RequestResponse::value_type &i, results)
    {
        requestResults.push_back(RequestResult(i.first->getEndpoint(),
                                               i.first->getRequest(),
                                               i.second.first,
                                               i.second.second));
    }

    try
    {
        m_handler(requestResults);
    }
    catch (const std::exception &e)
    {
        // TODO: detailed message
        Tools::Logger::Logger::getInstance().error(std::string("MultiRequestHandler() exception: ")
                + e.what());
    }
}

} /* namespace AsyncHttpClient */
} /* namespace Tools */
