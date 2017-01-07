#ifndef ERRORS_H_
#define ERRORS_H_

#include <stdexcept>

namespace Tools
{
namespace WebServer
{

class WebServerError : public std::runtime_error
{
public:
    explicit WebServerError(const std::string &message):
        std::runtime_error(message)
    {
    }

    virtual ~WebServerError() 
    {
    }
};

} /* namespace WebServer */
} /* namespace Tools */
#endif /* ERRORS_H_ */
