#ifndef TOOLS_LOGGER_LOGGERERRORS_H_
#define TOOLS_LOGGER_LOGGERERRORS_H_

#include <stdexcept>

namespace Tools
{
namespace Logger
{

/**
 * @brief Класс для оповещения об ошибках работы журнала сообщений
 */
class LoggerError : public std::runtime_error
{
public:

    explicit LoggerError(const std::string &message) :
            std::runtime_error(message)
    {
    }

    virtual ~LoggerError() 
    {
    }

};

} /* namespace Logger */
} /* namespace Tools */

#endif /* TOOLS_LOGGER_LOGGERERRORS_H_ */
