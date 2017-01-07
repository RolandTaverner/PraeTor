// C && C++ && POSIX
#include <map>
#include <string>

// BOOST
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

// Tools
#include "Tools/Configuration/XmlParser.h"
#include "Tools/Logger/Logger.h"
#include "Tools/MiscUtils/ThreadingModel.h"

namespace Conf = ::Tools::Configuration;
namespace Misc = ::Tools::MiscUtils;

namespace Tools
{
namespace Logger
{

//-----------------------------------------------------------------------------------------
Logger &Logger::getInstance(const std::string &name) 
{
    try
    {
        Misc::Locker<Misc::MultiThreading> locker(getThreadingModel());

        InstancesConstIterator it = getLoggerInstances().find(name);

        if (it == getLoggerInstances().end())
        {
            boost::shared_ptr<Logger> logger(new Logger());
            getLoggerInstances()[name] = logger;
            return *logger;
        }

        return *(it->second);
    }
    catch (LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Unknown error in Logger::getInstance()");
    }
}

bool Logger::existLogger(const std::string &name) 
{
    try
    {
        Misc::Locker<Misc::MultiThreading> locker(getThreadingModel());

        InstancesConstIterator it = getLoggerInstances().find(name);

        if (it == getLoggerInstances().end())
        {
            return false;
        }

        return true;
    }
    catch (LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Unknown error in Logger::getInstance()");
    }
}

//-----------------------------------------------------------------------------------------
void Logger::configure(const std::string &cfgFile, const std::string &section) 
{
    Conf::ConfigurationView config;
    try
    {
        config = Conf::Parsers::Xml::readXml(cfgFile).branch(section);
    }
    catch(std::exception &ex)
    {
        throw LoggerError(std::string("Error parse config file for Logger: ") + ex.what());
    }
    catch(...)
    {
        throw LoggerError("Error parse config file for Logger");
    }

    configure(config);
}

//-----------------------------------------------------------------------------------------
void Logger::configure(Conf::ConfigurationView logConfig) 
{
    try
    {
        Misc::Locker<Misc::MultiThreading> lockerThread(m_threadingModel);

        LoggerMessageLevel levels[] = {LDEBUG, LINFO, LTRACE, LWARNING, LERROR, LFATAL, LSTAT};
        const int numLevels = sizeof(levels) / sizeof(LoggerMessageLevel);

        /** Установка маски активных уровней сообщений **/
        if (logConfig.exists("active"))
        {
            Conf::ConfigurationView activeConfig = logConfig.branch("active");

            Conf::ConfigurationViewRange configs = activeConfig.getRangeOf("level");
            BOOST_FOREACH(Conf::ConfigurationView it, configs)
            {
                const std::string levelName = it.get("");

                if (levelName == "all")
                {
                    m_maskActiveLevel |= L_ALL;
                }
                else if (levelName == "debug")
                {
                    m_maskActiveLevel |= L_DEBUG;
                }
                else if (levelName == "info")
                {
                    m_maskActiveLevel |= L_INFO;
                }
                else if (levelName == "trace")
                {
                    m_maskActiveLevel |= L_TRACE;
                }
                else if (levelName == "warning")
                {
                    m_maskActiveLevel |= L_WARNING;
                }
                else if (levelName == "error")
                {
                    m_maskActiveLevel |= L_ERROR;
                }
                else if (levelName == "fatal")
                {
                    m_maskActiveLevel |= L_FATAL;
                }
                else if (levelName == "stat")
                {
                    m_maskActiveLevel |= L_STAT;
                }
                else
                {
                    throw LoggerError("Wrong configuration for active level");
                }
            }
        }
        else //if (logConfig.exists("active"))
        {
            m_maskActiveLevel = L_ALL;
        }

        /** Установка параметров работы для каждого уровня сообщений **/
        Conf::ConfigurationView levelConfig = logConfig.branch("level");
        if (levelConfig.exists("all"))
        {
            Conf::ConfigurationView anyConfig = levelConfig.branch("all");
            for (int i = 0; i < numLevels; ++i)
            {
                configureLevel(levels[i], anyConfig);
            }
        }

        for (int i = 0; i < numLevels; ++i)
        {
            if (levelConfig.exists(levels[i].getName()))
            {
                Conf::ConfigurationView config = levelConfig.branch(levels[i].getName());

                if (config.exists("all_add") && !config.get<bool>("all_add"))
                {
                    m_mapLevelStream[levels[i].getName()].clear();
                }

                configureLevel(levels[i], config);
            }
        }
    }
    catch (const LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Error configure Logger");
    }
}

//-----------------------------------------------------------------------------------------
template<class T>
int Logger::configureStream(const std::string &nameStream,
                            const LoggerMessageLevel &level,
                            Conf::ConfigurationView levelConfig,
                            T newStream) 
{
    try
    {
        Conf::ConfigurationViewRange configs = levelConfig.getRangeOf(nameStream);
        BOOST_FOREACH(Conf::ConfigurationView itStreamConf, configs)
        {
            std::string path;
            if ((nameStream == "stdout") || (nameStream == "stderr"))
            {
                path = nameStream;
            }
            else
            {
                path = itStreamConf.get("name");
            }

            if (path.empty())
            {
                continue;
            }

            std::map<std::string, LoggerBaseStreamPtr>::iterator itName;
            itName = m_mapResourceName.find(path);
            if (itName == m_mapResourceName.end())
            {
                boost::shared_ptr<LoggerBaseStream> pStream = newStream(itStreamConf);
                m_mapResourceName[path] = pStream;
                m_mapLevelStream[level.getName()].push_back(pStream);
            }
            else
            {
                boost::shared_ptr<LoggerBaseStream> pStream = itName->second;
                m_mapLevelStream[level.getName()].push_back(pStream);
            }
        }
    }
    catch (const LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Error configure streams of Logger");
    }
    return 0;
}

//-----------------------------------------------------------------------------------------
int Logger::configureLevel(const LoggerMessageLevel &level, Conf::ConfigurationView levelConfig)
        
{
    configureStream("file", level, levelConfig, LoggerFileStream::newStream);
    configureStream("stdout", level, levelConfig, LoggerStdOutStream::newStream);
    configureStream("stderr", level, levelConfig, LoggerStdErrStream::newStream);

    return 0;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::operator<<(const LoggerMessageLevel &messageLevel) 
{
    return LoggerStreamWriter(*this, messageLevel);
}

//-----------------------------------------------------------------------------------------
void Logger::debug(const std::string &message) 
{
    LoggerStreamWriter(*this, LDEBUG) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::debug() 
{
    return LoggerStreamWriter(*this, LDEBUG);
}

//-----------------------------------------------------------------------------------------
void Logger::debug(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LDEBUG);

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::info(const std::string &message) 
{
    LoggerStreamWriter(*this, LINFO) << message;
}

//-----------------------------------------------------------------------------------------
void Logger::trace(const std::string &message) 
{
    LoggerStreamWriter(*this, LTRACE) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::info() 
{
    return LoggerStreamWriter(*this, LINFO);
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::trace() 
{
    return LoggerStreamWriter(*this, LTRACE);
}

//-----------------------------------------------------------------------------------------
void Logger::info(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LINFO);

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::trace(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LTRACE);

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::warning(const std::string &message) 
{
    LoggerStreamWriter(*this, LWARNING) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::warning() 
{
    return LoggerStreamWriter(*this, LWARNING);
}

//-----------------------------------------------------------------------------------------
void Logger::warning(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LWARNING);

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::error(const std::string &message) 
{
    LoggerStreamWriter(*this, LERROR) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::error() 
{
    return LoggerStreamWriter(*this, LERROR);
}

//-----------------------------------------------------------------------------------------
void Logger::error(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LERROR); // m_threadingModel.lock();

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::fatal(const std::string &message) 
{
    LoggerStreamWriter(*this, LFATAL) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::fatal() 
{
    return LoggerStreamWriter(*this, LFATAL);
}

//-----------------------------------------------------------------------------------------
void Logger::fatal(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LFATAL); // m_threadingModel.lock();

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

//-----------------------------------------------------------------------------------------
void Logger::stat(const std::string &message) 
{
    LoggerStreamWriter(*this, LSTAT) << message;
}

//-----------------------------------------------------------------------------------------
LoggerStreamWriter Logger::stat() 
{
    return LoggerStreamWriter(*this, LSTAT);
}

//-----------------------------------------------------------------------------------------
void Logger::stat(const char *format, ...) 
{
    LoggerStreamWriter loggerStreamWriter(*this, LSTAT);

    va_list arguments;
    va_start(arguments, format);

    vsnprintf(m_sprintfBuffer, SPRINTF_BUFFER_SIZE, format, arguments);

    va_end(arguments);

    loggerStreamWriter << m_sprintfBuffer;
}

} //namespace Logger
} //namespace Tools

