#ifndef TOOLS_LOGGER_LOGGERHEADER_H
#define TOOLS_LOGGER_LOGGERHEADER_H

// C && C++ && POSIX
#include <exception>
#include <iostream>
#include <locale>
#include <memory>
#include <process.h>

// Tools
#include "Tools/Logger/LoggerBaseStream.h"

// BOOST
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

namespace Tools
{
namespace Logger
{

/**
 * @brief Интерфейс для классов, предназначенных для записи заголовков сообщений
 */

class ILoggerHeader
{
public:
    virtual ~ILoggerHeader() { };
    virtual void write(LoggerBaseStream &stream, const LoggerMessageLevel &messageLevel)  = 0;

    /**
     * Фабрика по производству заголовков сообщений различного типа
     * @param loggerHeaderName - имя необходимого типа заголовка
     * "0" или "empty" - заголовок отсутствует
     * "1" или "short" -заголовок с форматом вывода TIMESTAMP [LEVEL]: message - пример (2014-04-03T12:25:56.797428 [INFO]: message)
     * "2" или "full" -заголовок с форматом вывода TIMESTAMP hostname pid/tid [LEVEL]: message - пример (2014-04-03T12:25:56.797428 desktop 123/123456 [INFO]: message)
     * @return возвращает указатель на базовый класс всех типов заголовков
     * @exeption LoggerError - в случае неверного имени заголовка
     */
    static ILoggerHeader *newLoggerHeader(const std::string &loggerHeaderName);
};

/**
 * @brief Класс для записи заголовков сообщений
 * Формат вывода: TIMESTAMP [LEVEL]: message
 * @code
 * 2014-04-03T12:25:56.797428 [INFO]: message
 * @endcode
 */
class LoggerDefaultHeader : public ILoggerHeader
{
public:
    /**
     * Создание объекта, осуществляющего запись заголовков сообщений
     */
    LoggerDefaultHeader() 
    {
        try
        {
            boost::posix_time::time_facet *output_facet = new boost::posix_time::time_facet();
            output_facet->format("%Y-%m-%dT%H:%M:%S.%f");
            m_timeStampStream.imbue(std::locale(std::locale::classic(), output_facet));
        }
        catch(...)
        {
            throw LoggerError("Error create LoggerDefaultHeader");
        }
    }

    /**
     * Осущесвляет запись заголовков сообщений
     * @param stream - поток вывода сообщений
     * @param messageLevel - уровень вывода сообщений
     */
    void write(LoggerBaseStream &stream, const LoggerMessageLevel &messageLevel) 
    {
        try
        {
            stream.setMessageLevel(messageLevel);

            const boost::posix_time::ptime now = boost::posix_time::microsec_clock().universal_time();

            m_timeStampStream.clear();
            m_timeStampStream.str("");
            m_timeStampStream << now;

            stream << m_timeStampStream.str() << " [" << messageLevel.getName() << "]: ";
        }
        catch(LoggerError &)
        {
            throw;
        }
        catch(...)
        {
            throw LoggerError("Error write LoggerDefaultHeader");
        }
    }

private:
    std::ostringstream m_timeStampStream;
};

/**
 * @brief Класс для записи расширенного заголовка сообщений
 * Формат вывода: TIMESTAMP hostname pid/tid [LEVEL]: message
 * @code
 * 2014-04-03T12:25:56.797428 desktop 123/123456 [INFO]: message
 * @endcode
 */
class LoggerFullHeader : public ILoggerHeader
{
public:
    /**
     * @brief Конструктор LoggerFullHeader
     * Инициализирует формат вывода времени, запоминает хост текущей ЭВМ и pid процесса
     */
    LoggerFullHeader() 
    {
        try
        {
            // Создаем формат времени для заголовка
            boost::posix_time::time_facet *output_facet = new boost::posix_time::time_facet();
            output_facet->format("%Y-%m-%dT%H:%M:%S.%f");
            m_timeStampStream.imbue(std::locale(std::locale::classic(), output_facet));

            m_hostname = getHostName();
            m_pid = _getpid();
        }
        catch(...)
        {
            throw LoggerError("Error create LoggerDefaultHeader");
        }
    }

    /**
     * Выдает название хоста на котором выполняется процесс
     */
    std::string getHostName()
    {
        try
        {
            return boost::asio::ip::host_name();
        }
        catch(...)
        {
            return "Unknown host";
        }
    }

    /**
     * Осущесвляет запись заголовков сообщений
     * @param stream - поток вывода сообщений
     * @param messageLevel - уровень вывода сообщений
     */
    void write(LoggerBaseStream &stream, const LoggerMessageLevel &messageLevel) 
    {
        try
        {
            boost::thread::id threadId = boost::this_thread::get_id();

            stream.setMessageLevel(messageLevel);

            const boost::posix_time::ptime now = boost::posix_time::microsec_clock().universal_time();

            m_timeStampStream.clear();
            m_timeStampStream.str("");
            m_timeStampStream << now;

            stream  << m_timeStampStream.str() << " " << m_hostname <<" " << m_pid << "/" << threadId
                    << " [" << messageLevel.getName() << "]: ";
        }
        catch(LoggerError &)
        {
            throw;
        }
        catch(...)
        {
            throw LoggerError("Error write LoggerDefaultHeader");
        }
    }

private:
    std::ostringstream m_timeStampStream;  //!< Формат времени заголовка
    std::string m_hostname; //!< Название хоста на котором выполняется процесс
    int m_pid;  //!< Pid процесса
};

/**
 * @brief Класс для записи пустых заголовков сообщений
 */
class LoggerEmptyHeader : public ILoggerHeader
{
public:
    /**
     * @brief Пустая заглушка для записи пустых заголовков сообщений
     * @param stream - поток вывода сообщений
     * @param messageLevel - уровень вывода сообщений
     */
    void write(LoggerBaseStream &stream, const LoggerMessageLevel &messageLevel) 
    {
        stream.setMessageLevel(messageLevel);
    }
};

inline ILoggerHeader *ILoggerHeader::newLoggerHeader(const std::string &loggerHeaderName)
{
    if (loggerHeaderName == "0" || loggerHeaderName == "empty")
    {
        return new LoggerEmptyHeader();
    }
    else if (loggerHeaderName == "1" || loggerHeaderName == "short")
    {
        return new LoggerDefaultHeader();
    }
    else if (loggerHeaderName == "2" || loggerHeaderName == "full")
    {
        return new LoggerFullHeader();
    }
    else
    {
        throw LoggerError("Wrong name of logger header: " + loggerHeaderName);
    }
}

} //namespace Logger
} //namespace Tools

#endif //TOOLS_LOGGER_LOGGERHEADER_H
