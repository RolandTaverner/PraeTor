#ifndef TOOLS_LOGGER_LOGGERFILESTREAM_H
#define TOOLS_LOGGER_LOGGERFILESTREAM_H

// C++
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>

// Boost
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

// Tools
#include "Tools/Logger/LoggerBaseStream.h"
#include "Tools/Logger/LoggerHeader.h"

namespace Tools
{
namespace Logger
{

/**
 * @brief Файловый поток вывода сообщений
 * Класс определяет поток вывода сообщений в файл
 */
class LoggerFileStream : public LoggerBaseStream
{
public:

    /**
     * @brief Перечисление режимов ротирования файлов потока
     */
    enum RotateMode
    {
        RM_NULL = 0,  //!< Ротирование файлов отсутствует
        RM_SIZE,    //!< Ротирование файлов, согласно заданному порогу размера файло
        RM_PERIOD   //!< Ротирование через заданные промежутки времени
    };

    /**
     * Создает файловый поток с заданными параметрами работы
     * @param fileConfig - параметры работы файлового потока
     * @return указатель на созданный файловый поток
     * @exception LoggerError - ошибка создания файлового потока
     */
    static LoggerBaseStreamPtr newStream(const ::Tools::Configuration::ConfigurationView &fileConfig) ;

    /**
     * Создает объект файлового потока
     */
    LoggerFileStream();

    /**
     * Уничтожает объект
     */
    ~LoggerFileStream() ;

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
     * Открывает поток с заданным именем файла
     * @param fileName - имя файла
     * @param mode - режим открытия файла
     * @exception LoggerError - ошибка при открытии файла потока
     */
    void setFileName(const std::string &fileName,
                     std::ios_base::openmode mode = std::ios::out | std::ios::app)  ;

    /**
     * Осущесвляет установку параметров работы потока
     * @param fileConfig - параметры работы потока вывода сообщений
     * @exception LoggerError - ошибка установки параметров работы потока
     */
    void configure(const ::Tools::Configuration::ConfigurationView &fileConfig) ;

    /**
     * Записывает заголовок сообщения
     */
    void writeHeader(const LoggerMessageLevel &messageLevel) ;

    /**
     * Потоковый оператор вывода объекта в файловый поток вывода сообщений
     * @param out - объект вывода
     * @tparam T - тип объекта вывода
     * @exception LoggerError - ошибка записи объекта в поток
     */
    template <class T>
    LoggerFileStream &operator<<(const T &out) 
    {
        m_file.clear();
        m_file << out;
        if (!m_file)
        {
            const std::string error("Can't write to log file: " + m_currentFile);
            onError(error, m_errorMode);
        }
        return *this;
    }

    /**
     * Записывает сообщение в файловый поток вывода сообщений
     * @param out - сообщение для записи
     * @exception LoggerError - ошибка записи сообщения в поток
     */
    LoggerFileStream &write(const std::string &out) ;

    /**
     * Устанавливает уровень вывода в поток
     * @param LoggerMessageLevel - уровень вывода в поток
     */
    LoggerFileStream &setMessageLevel(const LoggerMessageLevel &) ;

private:
    bool doRotation();

    /**
     * создаем начальный список файлов
     */
    void creationBeginListFile(const boost::filesystem::path &directory,
    		                   const std::string &fileTemplate,
    		                   const size_t maxNumberFileInList) ;
private:
    std::ofstream m_file;
    std::string m_currentFile;

    boost::shared_ptr<ILoggerHeader> m_pHeader; //!< указатель на заголовок сообщения

    long m_size;
    RotateMode m_rotateMode;
    time_t m_periodSeconds;
    time_t m_nextSeconds;

    std::ostringstream m_timeStampStream;
    boost::posix_time::time_facet *m_pOutputFacet;

    typedef boost::filesystem::path PathToFile;
    typedef std::queue<PathToFile> Files;
    Files m_files;
    const static size_t m_maxNumberFileDefault = 30;
    size_t m_maxNumberFile;
};
//LoggerFileStream

} // namespace Logger
} // namespace Tools

#endif /* TOOLS_LOGGER_LOGGERFILESTREAM_H*/

