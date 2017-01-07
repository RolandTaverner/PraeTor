#ifndef IWEBSERVICE_H_
#define IWEBSERVICE_H_

// BOOST
#include <boost/shared_ptr.hpp>

#include "Tools/WebServer/ConnectionContext.h"

namespace Tools
{
namespace WebServer
{

class IWebService
{
public:
    virtual ~IWebService()
    {
    }

    virtual void operator()(ConnectionContextPtr contextPtr) = 0;
    virtual void start(void) = 0;
    virtual void stop(void) = 0;
};

typedef boost::shared_ptr<IWebService> IWebServicePtr;

} /* namespace WebServer */
} /* namespace Tools */

#endif /* IWEBSERVICE_H_ */
