/**
 * @file LoggerPipeStream.h
 * @author Andrey Karmazin, Sergey Gordeev, Alexey Karpov
 * @brief Поток вывода сообщений в программный канал
 */

#ifndef SIPSLIBRARIES_LOGGER_LOGGERPIPESTREAM_H
#define SIPSLIBRARIES_LOGGER_LOGGERPIPESTREAM_H

// POSIX
#include <fcntl.h>
#include <sys/stat.h>

// C
#include <errno.h>
#include <signal.h>

// C++
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>

// Boost
#include "boost/thread.hpp"
#include "boost/thread/condition.hpp"
#include "boost/thread/mutex.hpp"

// Tools
#include "Tools/Logger/LoggerBaseStream.h"
#include "Tools/Logger/LoggerHeader.h"

namespace Tools
{
namespace Logger
{

/**
 * @brief Поток вывода сообщений в программный канал
 * Класс определяет поток вывода сообщений в программный канал
 */
class LoggerPipeStream : public LoggerBaseStream
{
public:

    /**
     * Создает поток с заданными параметрами работы
     * @param pipeConfig - параметры работы  потока
     * @return указатель на созданный  поток
     * @exception LoggerError - ошибка создания потока
     */
    static  boost::shared_ptr<LoggerBaseStream> newStream(const ::Tools::Configuration::ConfigurationView &pipeConfig) ;

    /**
     * Создает объект вывода сообщений в программный канал
     *  @exception LoggerError - ошибка создания потока
     */
    LoggerPipeStream() ;

    /**
     * Уничтожает объект
     */
    ~LoggerPipeStream() ;

    /**
     * Осущесвляет сброс потока
     */
    void flush() ;

    /**
     * Осуществляет ротирование ресурсов внутри потока вывода сообщений
     * @exception LoggerError - ошибка в процессе ротирования ресурса потока
     */
    void rotate() ;

    /**
     * Определяет является ли указанный путь программным каналом
     */
    bool isFifo(const std::string &path) ;


    /**
     * Поток записи данных в программный канал
     */
    void thread();

    /**
     * Открывает поток с заданным именем файла
     * @param fileName - имя файла
     * @param mode - режим открытия файла
     * @exception LoggerError - ошибка при открытии файла потока
     */
    void setFileName(const std::string &fileName,
                     std::ios_base::openmode mode = std::ios::out | std::ios::app) ;


    /**
     * Осущесвляет установку параметров работы потока
     * @param pipeConfig - параметры работы потока
     * @exception LoggerError - ошибка установки параметров работы потока
     */
    void configure(const ::Tools::Configuration::ConfigurationView &pipeConfig) ;

    /**
     * Записывает заголовок сообщения
     */
    void writeHeader(const LoggerMessageLevel &messageLevel) ;

    /**
     * Потоковый оператор вывода объекта в поток вывода сообщений
     * @param out - объект вывода
     * @tparam T - тип объекта вывода
     * @exception LoggerError - ошибка записи объекта в поток
     */
    template <class T>
    LoggerPipeStream &operator<<(const T &out) 
    {
        try
        {
            boost::mutex::scoped_lock lock(m_mutex);
            if (m_pd >= 0)
            {
                if (out == "\n")
                {
                    m_sendBuffer += m_mesage;
                    m_sendBuffer += out;
                    m_mesage.clear();
                }
                else
                {
                    m_mesage += out;
                    return *this;;
                }

                if (m_sendBuffer.size() >= m_bufferSize)
                {
                    m_file.clear();
                    if (m_file.is_open())
                    {
                        size_t pos = m_sendBuffer.find_last_of("\n");

                        if (pos != std::string::npos)
                        {
                            m_file.write(m_sendBuffer.c_str(), pos + 1);
                            m_sendBuffer.erase(0, pos + 1);
                            m_file.flush();
                        }
                    }
                    else
                    {
                        m_sendBuffer.clear();
                    }
                    onError("Overflow data buff for pipe", EM_STDOUT);
                }

                if (m_wait)
                {
                    m_condition.notify_all();
                }
            } // if pd >=0;
            return *this;
        }
        catch (const LoggerError &e)
        {
            onError(e.what(), m_errorMode);
            return *this;
        }
        catch (...)
        {
            onError("Error write to pipe stream", m_errorMode);
            return *this;
        }
    }

    /**
     * Записывает сообщение в поток вывода сообщений
     * @param out - сообщение для записи
     * @exception LoggerError - ошибка записи сообщения в поток
     */
    LoggerPipeStream &write(const std::string &out) ;

    /**
     * Устанавливает уровень вывода в поток
     * @param LoggerMessageLevel - уровень вывода в поток
     */
    LoggerPipeStream &setMessageLevel(const LoggerMessageLevel &) ;

private:

    boost::shared_ptr<ILoggerHeader> m_pHeader; //!< указатель на заголовок сообщения

    std::ofstream m_file;
    volatile int m_pd;
    ulong m_bufferSize;
    std::string m_strbuffer;

    std::string m_sendBuffer;
    std::string m_mesage;
    std::string m_pipeBuffer;

    boost::thread m_thread;
    boost::mutex m_mutex;
    boost::condition m_condition;
    bool m_wait;

    size_t sendSize;
}; // LoggerPipeStream


} // namespace Logger
} // namespace Tools

#endif /* SIPSLIBRARIES_LOGGER_LOGGERPIPESTREAM_H */

