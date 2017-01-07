#ifndef TOOLS_LOGGER_LOGGERBASESTREAM_H
#define TOOLS_LOGGER_LOGGERBASESTREAM_H

// C && C++ && POSIX
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

// BOOST
#include <boost/shared_ptr.hpp>

// Tools
#include "Tools/Configuration/ConfigurationView.h"
#include "Tools/Logger/LoggerErrors.h"
#include "Tools/Logger/LoggerMessageLevel.h"

namespace Tools
{
namespace Logger
{

/**
 * @brief Интерфейс для потоков вывода сообщений
 */
class LoggerBaseStream
{
public:

    /**
     * @brief Перечисление режимов обработки ошибок для потоков вывода сообщений
     */
    enum ErrorMode
    {
        EM_NONE,        //!< Ошибки не обрабатываются
        EM_EXCEPTION,   //!< При возникновении ошибок генерируются исключения
        EM_STDOUT,      //!< Сообщения об ошибках выдаются на экран
        EM_STDERR       //!< Сообщения об ошибках выдаются на экран
    };

    /**
     * Создает объект потока вывода сообщений
     */
    LoggerBaseStream()  : m_errorMode(EM_STDERR)
    {
    }

    /**
     * Уничтожает объект потока вывода сообщений
     */
    virtual ~LoggerBaseStream() 
    {
    }

    /**
     * Осущесвляет сброс потока вывода сообщений
     */
    virtual void flush()  = 0;

    /**
     * Осуществляет ротирование ресурсов внутри потока вывода сообщений
     * @exception LoggerError - ошибка в процессе ротирования ресурсов потока
     */
    virtual void rotate()  = 0;

    /**
     * Открывает ресурсы потока с заданным именем файла
     * @param fileName - имя файла
     * @param mode - режим открытия файла
     * @exception LoggerError - ошибка при открытии ресурса потока
     */
    virtual void setFileName(const std::string &fileName, std::ios_base::openmode mode = std::ios::out | std::ios::app)   = 0;

    /**
     * Осущесвляет установку конфигурационных параметров работы потока
     * @param config - параметры работы потока вывода сообщений
     * @exception LoggerError - ошибка установки параметров работы потока
     */
    virtual void configure(const ::Tools::Configuration::ConfigurationView &config)  = 0;

    /**
     * Записывает заголовок сообщения
     * @param LoggerMessageLevel - уровень вывода в поток
     * @exception LoggerError - ошибка в процессе записи заголовка сообщений
     */
    virtual void writeHeader(const LoggerMessageLevel &messageLevel)  = 0;

    /**
     * Потоковый оператор вывода объекта в поток вывода сообщений
     * @param out - объект вывода
     * @tparam T - тип объекта вывода
     * @exception LoggerError - ошибка записи объекта в поток
     */
    template <typename T>
    LoggerBaseStream &operator<<(const T &out) 
    {
        try
        {
            m_bufferStream.clear();
            m_bufferStream.str("");

            m_bufferStream << out;

            std::string hi;

            if (m_bufferStream)
            {
                write(m_bufferStream.str());
                return *this;
            }
            else
            {
                onError("Error write to string buffer of logger base stream", m_errorMode);
                return *this;
            }
        }
        catch(LoggerError &e)
        {
            onError(e.what(), m_errorMode);
            return *this;
        }
        catch(...)
        {
            onError("Error write to logger base stream", m_errorMode);
            return *this;
        }
    }

    /**
     * Записывает сообщение в поток вывода сообщений
     * @param out - сообщение для записи
     * @exception LoggerError - ошибка записи сообщения в поток
     */
    virtual LoggerBaseStream &write(const std::string  &out)  = 0;

    /**
     * Устанавливает уровень вывода в поток
     * @param LoggerMessageLevel - уровень вывода в поток
     */
    virtual LoggerBaseStream &setMessageLevel(const LoggerMessageLevel &level)  = 0;

protected:

    ErrorMode m_errorMode;                //!< Режим обработки ошибок
    std::ostringstream m_bufferStream;    //!< Строковый буфер сообщения

    void setErrorMode(const std::string &errorMode)
    {
        if (errorMode == "exception")
        {
            m_errorMode = EM_EXCEPTION;
        }
        else if (errorMode == "stdout")
        {
            m_errorMode = EM_STDOUT;
        }
        else if (errorMode == "no")
        {
            m_errorMode = EM_NONE;
        }
        else if (errorMode == "stderr")
        {
            m_errorMode = EM_STDERR;
        }
        else
        {
            onError("Wrong value for section 'error'", m_errorMode);
        }
    }

    /**
     * Обработчик ошибок при работе потоков вывода сообщений
     * @param errorString - сообщение об ощибке
     * @param errorMode -  режим обработки ошибок
     * @exception LoggerError - генерируемое исключение об ошибке
     */
    void onError (const std::string &errorString, ErrorMode errorMode)  
    {
        switch (errorMode)
        {
            case EM_EXCEPTION:
                throw LoggerError(errorString);
                break;

            case EM_STDOUT:
                std::cout << errorString << std::endl;
                break;

            case EM_STDERR:
                std::cerr << errorString << std::endl;
                break;

            default:
                break;
        }
    }
}; // class ILoggerBaseStream

typedef boost::shared_ptr<LoggerBaseStream> LoggerBaseStreamPtr;

} // namespace Logger
} // namespace Tools

#endif /* TOOLS_LOGGER_LOGGERBASESTREAM_H */

