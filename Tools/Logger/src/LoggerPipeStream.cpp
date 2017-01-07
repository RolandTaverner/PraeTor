/**
 * @file LoggerPipeStream.cpp
 */

#include "SipsLibraries/Logger/LoggerPipeStream.h"

namespace Conf = ::SipsLibraries::Configuration;

namespace SipsLibraries
{
namespace Logger
{

LoggerBaseStreamPtr LoggerPipeStream::newStream(const Conf::ConfigurationView &pipeConfig)
        throw(LoggerError)
{
    try
    {
        LoggerBaseStreamPtr pPipe(new LoggerPipeStream);
        pPipe->configure(pipeConfig);
        return pPipe;
    }
    catch (const std::bad_alloc &)
    {
        throw LoggerError("Bad alloc memory for LoggerPipeStream");
    }
    catch (const LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Error create LoggerPipeStream");
    }
    return LoggerBaseStreamPtr();
}

//--------------------------------------------------------------------------------------------------
LoggerPipeStream::LoggerPipeStream() throw(LoggerError)
{
    m_pd = -1;
    m_bufferSize = 10 * 1024u * 1024u;
    m_wait = true;
    sendSize = 0;
}

//--------------------------------------------------------------------------------------------------
LoggerPipeStream::~LoggerPipeStream() throw()
{
    {
        boost::mutex::scoped_lock lock(m_mutex);
        if (m_pd >= 0)
        {
            close(m_pd);
            m_pd = -1;
        }
    }


    m_thread.interrupt();
    if (m_thread.joinable())
    {
        m_thread.join();
    }


}

//--------------------------------------------------------------------------------------------------
void LoggerPipeStream::flush() throw()
{
    static std::string strend = "\n";
    try
    {
        if (m_pd >= 0)
        {
            operator<<(strend);
        }
    }
    catch (...)
    {
        ;
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerPipeStream::rotate() throw(LoggerError)
{
}

//--------------------------------------------------------------------------------------------------
bool LoggerPipeStream::isFifo(const std::string &path) throw()
{
    struct stat statFile;

    if (lstat(path.c_str(), &statFile) != 0)
    {
        return false;
    }

    if (S_ISFIFO(statFile.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
void LoggerPipeStream::setFileName(const std::string &fileName, std::ios_base::openmode)
        throw(LoggerError)
{
    try
    {
        if (fileName.empty())
        {
            onError("Empty path for pipe", m_errorMode);
        }

        if (m_pd >= 0)
        {
            close(m_pd);
            m_pd = -1;
        }

        if ((mkfifo(fileName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0) && (errno != EEXIST))
        {
            const std::string error("Error access at path: " + fileName);
            onError(error, m_errorMode);
            return;
        }

        if (!isFifo(fileName))
        {
            const std::string error("Path isn't pipe: " + fileName);
            onError(error, m_errorMode);
            return;
        }

        const int pdr = open(fileName.c_str(), (O_RDONLY | O_NONBLOCK));
        if (pdr == (-1))
        {
            const std::string error("Error open pipe file for read: " + fileName);
            onError(error, m_errorMode);
            return;
        }

        m_pd = open(fileName.c_str(), (O_WRONLY  | O_APPEND));
        if (m_pd == (-1))
        {
            const std::string error("Error open pipe file for write: " + fileName);
            onError(error, m_errorMode);
            return;
        }

        signal(SIGPIPE, SIG_IGN );
        close(pdr);
    }
    catch (const LoggerError &e)
    {
        onError(e.what(), m_errorMode);
    }
    catch (...)
    {
        const std::string error("Error setFileName for pipe stream" + fileName);
        onError(error, m_errorMode);
    }
}

void LoggerPipeStream::thread()
{
    try
    {
        while (m_pd >= 0)
        {
            {
                boost::mutex::scoped_lock lock(m_mutex);

                while (m_sendBuffer.empty() && m_pipeBuffer.empty())
                {
                    m_wait = true;
                    m_condition.wait(lock);
                }

                m_wait = false;
                m_pipeBuffer += m_sendBuffer;
                m_sendBuffer.clear();
            }

            while (m_pd >= 0)
            {
                if (m_pipeBuffer.empty())
                {
                    break;
                }

                const int res = ::write(m_pd, m_pipeBuffer.c_str(), m_pipeBuffer.size());

                if (res < 0)
                {
                    if (errno == EPIPE) // Никто не подсоединен к каналу для чтения
                    {
                        ::sleep(1);
                      //std::cout << "LogService don't read pipe" << stream.str().size() << std::endl;
                    }
                    else if (errno == EAGAIN) // Канал полностью полон
                    {
                        //std::cout << "No space in pipe"  << std::endl;
                    }
                    else if (errno == EINTR) // Канал полностью полон
                    {
                        //std::cout << "Signal interrupted write to pipe" << stream.str().size() << std::endl;
                    }
                    else // Ошибка записи в канал
                    {

                        onError("Error write to pipe", m_errorMode);
                    }

                    continue;
                }

                if ((unsigned)res != m_pipeBuffer.size())
                {
                    m_pipeBuffer.erase(0, res);
                    sendSize += res;
                    continue;
                }

                sendSize += res;
                m_pipeBuffer.clear();
            }
        }
    }
    catch(...)
    {
        ;
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerPipeStream::configure(const Conf::ConfigurationView &pipeConfig) throw(LoggerError)
{
    try
    {
        m_errorMode = EM_EXCEPTION;

        setFileName(pipeConfig.get("name"));

        m_thread = boost::thread(&LoggerPipeStream::thread, this);

        /** Установка типа заголовка сообщений **/
        m_pHeader.reset(ILoggerHeader::newLoggerHeader(pipeConfig.get<std::string>("header", "short")));

        if (pipeConfig.exists("file"))
        {
            const std::string fileName = pipeConfig.get("file");
            if (fileName.empty())
            {
                onError("Empty path for file", m_errorMode);
                return;
            }

            m_file.clear();
            if (m_file.is_open())
            {
                m_file.close();
            }

            m_file.clear();
            m_file.open(fileName.c_str(), std::ios::out | std::ios::app);
            if (m_file.is_open())
            {
                //std::cout << "\tOpen file for error backup: " << fileName << std::endl;
            }
            else
            {
                const std::string error("Can't open file: " + fileName);
                onError(error, m_errorMode);
            }
        }

        if (pipeConfig.exists("buffer"))
        {
            m_bufferSize = pipeConfig.get<long>("buffer") * 1024;
        }


        //std::cout << "\tBuffer for pipe: " << m_bufferSize << std::endl;

        const std::string errorString = pipeConfig.get("error", std::string("stderr"));
        setErrorMode(errorString);
    }
    catch (const LoggerError &e)
    {
        onError(e.what(), m_errorMode);
    }
    catch (...)
    {
        onError("Error configure for pipe stream", m_errorMode);
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerPipeStream::writeHeader(const LoggerMessageLevel &messageLevel) throw(LoggerError)
{
    if (m_pHeader)
    {
        m_pHeader->write(*this, messageLevel);
    }
}

//--------------------------------------------------------------------------------------------------
LoggerPipeStream &LoggerPipeStream::write(const std::string &out) throw(LoggerError)
{
    operator<<(out);
    return *this;
}

//--------------------------------------------------------------------------------------------------
LoggerPipeStream &LoggerPipeStream::setMessageLevel(const LoggerMessageLevel &) throw()
{
    return *this;
}

} /* namespace Logger */
} /* namespace SipsLibraries */

