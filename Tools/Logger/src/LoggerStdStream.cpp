#include "Tools/Logger/LoggerStdStream.h"

namespace Conf = ::Tools::Configuration;

namespace Tools
{
namespace Logger
{

//--------------------------------------------------------------------------------------------------
LoggerBaseStreamPtr LoggerStdOutStream::newStream(const Conf::ConfigurationView &stdOutConfig) 
{
    try
    {
        LoggerBaseStreamPtr pStdOut(new LoggerStdOutStream());
        pStdOut->configure(stdOutConfig);
        return pStdOut;
    }
    catch (std::bad_alloc)
    {
        throw LoggerError("Bad alloc memory for LoggerStdOutStream");
    }
    catch (...)
    {
        throw LoggerError("Error create LoggerStdOutStream");
    }
    return LoggerBaseStreamPtr();
}

//--------------------------------------------------------------------------------------------------
LoggerStdOutStream::LoggerStdOutStream() 
{
}

//--------------------------------------------------------------------------------------------------
LoggerStdOutStream::~LoggerStdOutStream() 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdOutStream::flush() 
{
    std::cout << std::endl;
}

//--------------------------------------------------------------------------------------------------
void LoggerStdOutStream::rotate() 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdOutStream::setFileName(const std::string &, std::ios_base::openmode) 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdOutStream::configure(const Conf::ConfigurationView &stdOutConfig) 
{
    /** Установка типа заголовка сообщений **/
    m_pHeader.reset(ILoggerHeader::newLoggerHeader(stdOutConfig.get<std::string>("header", "short")));
}

//--------------------------------------------------------------------------------------------------
void LoggerStdOutStream::writeHeader(const LoggerMessageLevel &messageLevel) 
{
    if (m_pHeader)
    {
        m_pHeader->write(*this, messageLevel);
    }
}

//--------------------------------------------------------------------------------------------------
LoggerStdOutStream &LoggerStdOutStream::write(const std::string &out) 
{
    operator<<(out);
    return *this;
}

//--------------------------------------------------------------------------------------------------
LoggerStdOutStream &LoggerStdOutStream::setMessageLevel(const LoggerMessageLevel &) 
{
    return *this;
}

//--------------------------------------------------------------------------------------------------
LoggerBaseStreamPtr LoggerStdErrStream::newStream(const Conf::ConfigurationView &stdErrConfig) 
{
    try
    {
        LoggerBaseStreamPtr pStdErr(new LoggerStdErrStream);
        pStdErr->configure(stdErrConfig);
        return pStdErr;
    }
    catch (std::bad_alloc)
    {
        throw LoggerError("Bad alloc memory for LoggerStdErrStream");
    }
    catch (...)
    {
        throw LoggerError("Error create LoggerStdErrStream");
    }
    return LoggerBaseStreamPtr();
}

//--------------------------------------------------------------------------------------------------
LoggerStdErrStream::LoggerStdErrStream() 
{
}

//--------------------------------------------------------------------------------------------------
LoggerStdErrStream::~LoggerStdErrStream() 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdErrStream::flush() 
{
    std::cerr << std::endl;
}

//--------------------------------------------------------------------------------------------------
void LoggerStdErrStream::rotate() 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdErrStream::setFileName(const std::string &, std::ios_base::openmode) 
{
}

//--------------------------------------------------------------------------------------------------
void LoggerStdErrStream::configure(const Conf::ConfigurationView &stdErrConfig) 
{
    /** Установка типа заголовка сообщений **/
    m_pHeader.reset(ILoggerHeader::newLoggerHeader(stdErrConfig.get<std::string>("header", "short")));
}

//--------------------------------------------------------------------------------------------------
void LoggerStdErrStream::writeHeader(const LoggerMessageLevel &messageLevel) 
{
    if (m_pHeader)
    {
        m_pHeader->write(*this, messageLevel);
    }
}

//--------------------------------------------------------------------------------------------------
LoggerStdErrStream &LoggerStdErrStream::write(const std::string &out) 
{
    operator<<(out);
    return *this;
}

//--------------------------------------------------------------------------------------------------
LoggerStdErrStream &LoggerStdErrStream::setMessageLevel(const LoggerMessageLevel &) 
{
    return *this;
}

} /* namespace Logger */
} /* namespace Tools */

