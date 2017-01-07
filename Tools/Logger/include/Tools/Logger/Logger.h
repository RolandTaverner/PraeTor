#ifndef TOOLS_LOGGER_LOGGER_H
#define TOOLS_LOGGER_LOGGER_H

// C++
#include <cstdarg>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

// Boost
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

// Tools
#include "Tools/Configuration/ConfigurationView.h"
#include "Tools/Logger/LoggerErrors.h"
#include "Tools/Logger/LoggerBaseStream.h"
#include "Tools/Logger/LoggerStdStream.h"
#include "Tools/Logger/LoggerFileStream.h"
#include "Tools/Logger/LoggerHeader.h"
#include "Tools/MiscUtils/ThreadingModel.h"

namespace Tools
{
namespace Logger
{

class LoggerStreamWriter;

/**
 * @brief Журнал сообщений
 *
 * Класс управляет выводом сообщений в журнал
 * Удобные макросы доступа:  GetLogger(name)
 *
 * Пример использования @ref Logger :
 * @code
 * #include "Logger.h"
 * ...
 * using namespace ::Tools::Configuration;
 * namespace Logger = ::Tools::Logger;
 *
 *
 *
 * Logger::Logger &logger = Logger::Logger::getInstance();
 * try
 * {
 *      logger("path_to_xml/name.xml", "config.logger"); //вариант инициализации 1
 * }
 * catch(Logger::LoggerError &e)
 * {
 * cout << "exception:" << e.what() << std::endl;
 * }
 * ...
 * logger << DEBUG << "debug debug debug";
 * logger.debug() << "debug " << "debug " << "debug " << 42;
 * logger.debug("debug debug debug");
 * logger.debug("%s %d", "debug debug debug", 42);
 *
 * logger << INFO << "info info info";
 * logger.info() << "info " << "info " << "info " << 42;
 * logger.info("info info info");
 * logger.info("%s %d", "info info info", 42);
 * ...
 * ConfigurationView root = Parsers::Xml::readXml("path_to_xml/name.xml");
 * ConfigurationView logConfig = root.branch("config.logger");
 * Logger::Logger &logger = Logger::Logger::getInstance("secondLogger");
 * try
 * {
 *      logger.configure(logConfig); //вариант инициализации 2
 * }
 * catch(Logger::LoggerError &e)
 * {
 *      cout << "exception:" << e.what() << std::endl;
 * }
 * ...
 * logger << WARNING << "warning warning warning";
 * logger.warning() << "warning " << "warning " << "warning " << 42;
 * logger.warning("warning warning warning");
 * logger.warning("%s %d", "warning warning warning", 42);
 *
 * logger << ERROR << "error error error";
 * logger.error() << "error " << "error " << "error " << 42;
 * logger.error("error error error");
 * logger.error("%s %d", "error error error", 42);
 *
 * logger << FATAL << "fatal fatal fatal";
 * logger.fatal() << "fatal " << "fatal " << "fatal " << 42;
 * logger.fatal("fatal fatal fatal");
 * logger.fatal("%s %d", "fatal fatal fatal", 42);
 * ...
 * @endcode
 *
 * Пример простого файла конфигурации. (Полная версия для всех частных случаев смотри $KMSEARCH_ROOT/Tools/trunk/Logger/doc/example.xml)

 Собщения имеют загоовок гггг-мм-ддTчч:мм:сс.xxxxx [Уровень]
 Для всех уровней - сообщения записываются в файл (ротирование отсутствует, ошибки выводятся на экран),
 в программный канал (буффер 1000кб, в случае его переполнения данные теряются, ошибки выводятся на экран),
 в стандартный поток вывода
 Для уровня FATAL сообщения дополнительно записываюся в еще один файл (ротирование отсутствует, ошибки выводятся на экран)
 * @code
<config>
    <logger>
        <level>
            <all>
                <file>
                    <name> /home/akarpov/file/file1 </name>
                </file>

                <pipe>
                    <name> /home/akarpov/file/pipe2 </name>
                </pipe>

                <stdout> </stdout>
            </all>

            <fatal>
                <file>
                    <name> /home/akarpov/file/file_error </name>
                </file>
            </fatal>
        </level>
    </logger>
</config>

 * @endcode
 *
 * @warning Максимальный размер сообщений при выводе в нотации printf ограничен @ref SPRINTF_BUFFER_SIZE
 * @note Потоковые операторы вывода сообщений в журнал не поддерживают стандартные манипуляторы (std::endl и т.п.).
 *       Перевод строки осуществляется автоматически.
 */

class Logger
{
    /** Итератор доступа к экземплярам */
    typedef std::map<std::string, boost::shared_ptr<Logger> >::const_iterator InstancesConstIterator;

public:

    Logger() :
            m_maskActiveLevel(0)
    {
    }

    /**
     * Возвращает ссылку на журнал сообщений с именем name
     * @param name - имя журнала сообщений
     * @return экземпляр объекта журнала сообщений
     * @exception LoggerError
     */
    static Logger &getInstance(const std::string &name = "default") ;

    static bool existLogger(const std::string &name) ;

    /**
     * Осущесвляет установку параметров работы журнала сообщений
     * @param logConfig - параметры работы журнала сообщений
     * @exception LoggerError - ошибка установки параметров работы журнала сообщений
     */
    void configure(::Tools::Configuration::ConfigurationView logConfig) ;

    /**
     * Осущесвляет установку параметров работы журнала сообщений
     * @param cfgFile - путь до конфигурационного файла
     * @param section - секция в файле, содержащая параметры журнала сообщений
     * @exception LoggerError - ошибка установки параметров работы журнала сообщений
     */
    void configure(const std::string &cfgFile, const std::string &section) ;

    /**
     * Поточный оператор вывода сообщения
     * @param messageLevel уровень сообщения
     * @return поток вывода сообщений
     */
    LoggerStreamWriter operator<<(const LoggerMessageLevel &messageLevel) ;

    /**
     * Осуществляет вывод отладочного сообщения
     * @param message - сообщение
     */
    void debug(const std::string &message) ;

    /**
     * @return поток вывода отладочного сообщения
     */
    LoggerStreamWriter debug() ;
    /**
     * Осуществляет вывод отладочного сообщения в нотации printf
     * @param format - формат
     */

    void debug(const char *format, ...) ;

    /**
     * Осуществляет вывод информационного сообщения
     * @param message - сообщение
     */
    void info(const std::string &message) ;

    /**
     * Осуществляет вывод трассировочного сообщения
     * @param message - сообщение
     */
    void trace(const std::string &message) ;

    /**
     * @return поток вывода информационного сообщения
     */
    LoggerStreamWriter info() ;

    /**
     * @return поток вывода трассировочного сообщения
     */
    LoggerStreamWriter trace() ;

    /**
     * Осуществляет вывод информационного сообщения в нотации printf
     * @param format - формат
     */
    void info(const char *format, ...) ;

    /**
     * Осуществляет вывод трассировочного сообщения в нотации printf
     * @param format - формат
     */
    void trace(const char *format, ...) ;

    /**
     * Осуществляет вывод предупреждения
     * @param message - предупреждение
     */
    void warning(const std::string &message) ;

    /**
     * @return поток вывода предупреждения
     */
    LoggerStreamWriter warning() ;

    /**
     * Осуществляет вывод предупреждения в нотации printf
     * @param format - формат
     */
    void warning(const char *format, ...) ;

    /**
     * Осуществляет вывод сообщения об ошибке
     * @param message - сообщение
     */
    void error(const std::string &message) ;

    /**
     * @return поток вывода сообщения об ошибке
     */
    LoggerStreamWriter error() ;

    /**
     * Осуществляет вывод сообщения об ошибке в нотации printf
     * @param format - формат
     */
    void error(const char *format, ...) ;

    /**
     * Осуществляет вывод сообщения о критической ошибке
     * @param message - сообщение
     */
    void fatal(const std::string &message) ;

    /**
     * @return поток вывода сообщения о критической ошибке
     */
    LoggerStreamWriter fatal() ;

    /**
     * Осуществляет вывод сообщения о критической ошибке в нотации printf
     * @param format - формат
     */
    void fatal(const char *format, ...) ;

    /**
     * Осуществляет вывод сообщения со статистической информацией
     * @param message - сообщение
     */
    void stat(const std::string &message) ;

    /**
     * @return поток вывода сообщения со статистической информацией
     */
    LoggerStreamWriter stat() ;

    /**
     * Осуществляет вывод сообщения со статистической информацией в нотации printf
     * @param format - формат
     */
    void stat(const char *format, ...) ;

private:

    /**
     * Осущесвляет создание и установку параметров работы для каждого типа потока вывода журнала сообщений
     * @param nameStream - имя потока вывода ("pipe", "file", "stdout", stderr)
     * @param LoggerMessageLevel - уровень сообщений
     * @param levelConfig - параметры работы потоков вывода
     * @param newStream - функция создания нового потока вывода
     * @tparam T - тип функции создания нового потока вывода
     * @exception LoggerError - ошибка создания и установки параметров работы потоков
     */
    template<class T>
    int configureStream(const std::string &nameResource,
                        const LoggerMessageLevel &level,
                        ::Tools::Configuration::ConfigurationView levelConfig,
                        T newStream) ;

    /**
     * Осуществляет создание и установку параметров работы пооков для каждого уровня сообщений журнала
     * @param LoggerMessageLevel - уровень сообщений
     * @param levelConfig - параметры работы потоков вывода
     */
    int configureLevel(const LoggerMessageLevel &level,
                       ::Tools::Configuration::ConfigurationView levelConfig) ;

    /** Статические методы для работы со статическими переменными класса Logger **/
    static Tools::MiscUtils::MultiThreading &getThreadingModel()
    {
        static Tools::MiscUtils::MultiThreading instanceThreadingMode;
        return instanceThreadingMode;
    }

    static std::map<std::string, boost::shared_ptr<Logger> > &getLoggerInstances()
    {
        static std::map<std::string, boost::shared_ptr<Logger> > instances;
        return instances;
    }

    /** Модель многопоточности журнала сообщений */
    ::Tools::MiscUtils::MultiThreading m_threadingModel;

    std::map<std::string, LoggerBaseStreamPtr> m_mapResourceName;
    std::map<std::string, std::vector<LoggerBaseStreamPtr> > m_mapLevelStream;

    /** Модель многопоточности для доступа к экземплярам журналов сообщений */
    //static ::Tools::MiscUtils::MultiThreading m_instanceThreadingModel;

    /** Экземпляры журналов сообщений */
    //static std::map<std::string, boost::shared_ptr<Logger> > m_instances;

    /** Размер буффера вывода сообщений в нотации printf */
    static const size_t SPRINTF_BUFFER_SIZE = 1024;

    /** Буффера вывода сообщений в нотации printf*/
    char m_sprintfBuffer[SPRINTF_BUFFER_SIZE];

    /** Маска активных уровней сообщений **/
    unsigned int m_maskActiveLevel;

    friend class LoggerStreamWriter;
};

/**
 * @brief Поточный "писатель"
 * Объект класса осуществляет непосредственную запись сообщения в потоки вывода журнала
 */
class LoggerStreamWriter
{
public:
    /**
     * Конструктор класса. Создает объект
     * @param logger - журнал сообщений
     */
    LoggerStreamWriter(Logger &logger, const LoggerMessageLevel &messageLevel)  :
            m_logger(logger), m_messageLevel(messageLevel), m_objectCopiesCount(0)

    {
        try
        {
            m_logger.m_threadingModel.lock();
            std::vector<LoggerBaseStreamPtr>::iterator it;

            if (m_messageLevel.getMask() & m_logger.m_maskActiveLevel)
            {
                for (it = m_logger.m_mapLevelStream[m_messageLevel.getName()].begin();
                     it != m_logger.m_mapLevelStream[m_messageLevel.getName()].end();
                     ++it)
                {
                    (*it)->rotate();
                    (*it)->writeHeader(messageLevel);
                }
            }

            m_objectCopiesCount = new size_t(0);
        }
        catch (LoggerError &)
        {
            m_logger.m_threadingModel.unlock();
            throw;
        }
        catch (...)
        {
            m_logger.m_threadingModel.unlock();
            throw LoggerError("Error create LoggerStreamWriter");
        }
    }

    /**
     * Уничтожает объект
     */
    ~LoggerStreamWriter() 
    {
        try
        {
            if (*m_objectCopiesCount)
            {
                --(*m_objectCopiesCount);
            }
            else
            {

                std::vector<LoggerBaseStreamPtr>::iterator it;

                if (m_messageLevel.getMask() & m_logger.m_maskActiveLevel)
                {

                    for (it = m_logger.m_mapLevelStream[m_messageLevel.getName()].begin();
                            it != m_logger.m_mapLevelStream[m_messageLevel.getName()].end(); ++it)
                    {
                        (*(*it)).flush();
                    }
                }

                m_logger.m_threadingModel.unlock();
                delete m_objectCopiesCount;
            }
        }
        catch (...)
        {
            ;
        }
    }

    /**
     * Поточный оператор вывода объекта
     * @param out - объект вывода
     * @param T - тип объекта вывода
     */
    template<class T>
    LoggerStreamWriter &operator<<(const T &out) 
    {
        try
        {
            if (!(m_messageLevel.getMask() & m_logger.m_maskActiveLevel))
                return *this;
            std::vector<LoggerBaseStreamPtr>::iterator it;

            for (it = m_logger.m_mapLevelStream[m_messageLevel.getName()].begin();
                    it != m_logger.m_mapLevelStream[m_messageLevel.getName()].end(); ++it)
            {
                (*(*it)) << out;
            }

            return *this;
        }
        catch (LoggerError &)
        {
            throw;
        }
        catch (...)
        {
            throw LoggerError("Error write LoggerStreamWriter");
        }
    }

private:
    /**
     * Запрещает копирование объекта
     */
    LoggerStreamWriter(const LoggerStreamWriter &other) :
            m_logger(other.m_logger), m_messageLevel(other.m_messageLevel), m_objectCopiesCount(
                    other.m_objectCopiesCount)
    {
        //std::cout << "Constructor copy LoggerStreamWriter" << std::endl;
        ++(*m_objectCopiesCount);
    }

    /**
     * Запрещает копирование объекта
     */
    const LoggerStreamWriter &operator=(const LoggerStreamWriter &);

    /** Журнал сообщений **/
    Logger &m_logger;

    /** Уровень сообщений **/
    const LoggerMessageLevel &m_messageLevel;

    /** Количество копий объекта */
    size_t *m_objectCopiesCount;

    /** Дружественный класс для осуществления копирования объекта */
    friend class Logger;
};

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// строка и источником сообщения (имя_файла:строка)
#define TAGET_MESSAGE \
        __FILE__ ":" TOSTRING(__LINE__)

} // namespace Logger
} // namespace Tools

#endif // TOOLS_LOGGER_LOGGER_H
