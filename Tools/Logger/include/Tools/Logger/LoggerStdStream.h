#ifndef TOOLS_LOGGER_LOGGERSTDSTREAM_H
#define TOOLS_LOGGER_LOGGERSTDSTREAM_H

// POSIX
#include <fcntl.h>
#include <sys/stat.h>

// C
#include <signal.h>

// C++
#include <cstdarg>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <vector>

// Boost
#include <boost/assert.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

// Tools
#include "Tools/Logger/LoggerBaseStream.h"
#include "Tools/Logger/LoggerHeader.h"

namespace Tools
{
namespace Logger
{

/**
 * @brief Поток вывода сообщений в stdout
 */
class LoggerStdOutStream : public LoggerBaseStream
{
public:

    /**
     * Динамическое создание нового потока вывода сообщений в стандартный поток вывода
     * @param stdOutConfig - параметры работы потока
     * @return - умный указатель на динамически созданный поток
     * @exception - LoggerError
     */
    static LoggerBaseStreamPtr newStream(const ::Tools::Configuration::ConfigurationView& stdOutConfig) ;

    /**
     * Создает объект
     */
    LoggerStdOutStream() ;

    /**
     * Уничтожает объект
     */
    ~LoggerStdOutStream() ;

    /**
     * Осущесвляет сброс потока
     */
    void flush() ;

    /**
     * Осущесвляет ротирование ресурсов внутри потока
     */
    void rotate() ;

    void setFileName(const std::string &fileName,
                     std::ios_base::openmode mode = std::ios::out | std::ios::app) ;

    /**
     * Осущесвляет настройку параметров работы потока
     */
    void configure(const ::Tools::Configuration::ConfigurationView &stdOutConfig) ;

    /**
     * Записывает заголовок сообщения
     */
    void writeHeader(const LoggerMessageLevel &messageLevel) ;

    /**
     * Потоковый оператор вывода объекта в файл
     * @param out - объект вывода
     * @tparam T - тип объекта вывода
     */
    template<class T>
    LoggerStdOutStream &operator<<(const T &out) 
    {
        std::cout << out;
        return *this;
    }

    /**
     * Потоковый оператор вывода объекта в файл
     * @param out - объект вывода
     */
    LoggerStdOutStream &write(const std::string &out) ;

    /**
     * Устанавливает уровень вывода в поток
     * @param LoggerMessageLevel - уровень вывода в поток
     */
    LoggerStdOutStream &setMessageLevel(const LoggerMessageLevel &) ;

private:
    boost::shared_ptr<ILoggerHeader> m_pHeader; //!< указатель на заголовок сообщения

};
//LoggerStdOutStream

/**
 * @brief Поток вывода сообщений в stderr
 */
class LoggerStdErrStream : public LoggerBaseStream
{
public:

    /**
     * Динамическое создание нового потока вывода сообщений в стандартный поток ошибок
     * @param stdErrConfig - параметры работы потока
     * @return - умный указатель на созданный поток
     * @exception - LoggerError
     */
    static LoggerBaseStreamPtr newStream(const ::Tools::Configuration::ConfigurationView &stdErrConfig) ;

    /**
     * Создает объект
     */
    LoggerStdErrStream() ;

    /**
     * Уничтожает объект
     */
    ~LoggerStdErrStream() ;

    /**
     * Осущесвляет сброс потока
     */
    void flush() ;

    /**
     * Осущесвляет ротирование ресурсов внутри потока
     */
    void rotate() ;

    void setFileName(const std::string &fileName,
                     std::ios_base::openmode mode = std::ios::out | std::ios::app) ;

    /**
     * Осущесвляет установку параметров работы потока
     */
    void configure(const ::Tools::Configuration::ConfigurationView &stdErrConfig) ;

    /**
     * Потоковый оператор вывода объекта в файл
     * @param out - объект вывода
     * @tparam T - тип объекта вывода
     */
    template<class T>
    LoggerStdErrStream &operator<<(const T &out) 
    {
        std::cerr << out;
        return *this;
    }

    /**
     * Записывает заголовок сообщения
     */
    void writeHeader(const LoggerMessageLevel &messageLevel) ;

    /**
     * Потоковый оператор вывода объекта в файл
     * @param out - объект вывода
     */
    LoggerStdErrStream &write(const std::string &out) ;

    /**
     * Устанавливает уровень вывода в поток
     * @param LoggerMessageLevel - уровень вывода в поток
     */
    LoggerStdErrStream &setMessageLevel(const LoggerMessageLevel &) ;

private:
    boost::shared_ptr<ILoggerHeader> m_pHeader; //!< указатель на заголовок сообщения
};
// LoggerStdErrStream

}// namespace Logger
} // namespace Tools

#endif /* TOOLS_LOGGER_LOGGERSTDSTREAM_H */

