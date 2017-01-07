#ifndef LOGGERMESSAGELEVEL_H_
#define LOGGERMESSAGELEVEL_H_

#include <string>

namespace Tools
{
namespace Logger
{

/** Битовые маски для уровней сообщений журнала **/
const unsigned int L_ALL         = 0xffff;
const unsigned int L_DEBUG       = 0x0001;
const unsigned int L_INFO        = 0x0002;
const unsigned int L_TRACE       = 0x0004;
const unsigned int L_WARNING     = 0x0008;
const unsigned int L_ERROR       = 0x0010;
const unsigned int L_FATAL       = 0x0020;
const unsigned int L_STAT        = 0x0040;

/**
 * @brief Уровень сообщений
 * Класс определяет уровень сообщений журнала
 * По умолчанию определенны следующие уровни:
 *  - @a LDEBUG
 *  - @a LINFO
 *  - @a LTRACE
 *  - @a LWARNING
 *  - @a LERROR
 *  - @a LFATAL
 *  - @a LSTAT
 */

class LoggerMessageLevel
{
public:
    /**
     * Создает объект для заданного уровня сообщений
     * @param name - имя уровня
     */
    LoggerMessageLevel(const std::string &name, unsigned int maskLevel = 0) ;

    /**
     * Возвращает имя уровня сообщений
     * @return имя уровня
     * */
    const std::string &getName() const ;

    /**
     * Возвращает битовую маску уровня сообщений
     * @return битовая маска уровня
     * */
    unsigned int getMask() const ;

/// @name Операторы сравнения объектов класса
/// @{
    bool operator== (const LoggerMessageLevel &right) const;

    bool operator!= (const LoggerMessageLevel &right) const;
/// @}

private:
    std::string m_name;         //!< Имя уровня
    unsigned int m_maskLevel;   //!< Маска уровня
}; // class LoggerMessageLevel

/** Уровни сообщений, определенные по умолчанию */
extern const LoggerMessageLevel LEMPTY;   //!< Пустой уровень
extern const LoggerMessageLevel LDEBUG;   //!< Отладочный уровень
extern const LoggerMessageLevel LINFO;    //!< Информационный уровень
extern const LoggerMessageLevel LTRACE;   //!< Трассировочный уровень
extern const LoggerMessageLevel LWARNING; //!< Уровень предупреждений
extern const LoggerMessageLevel LERROR;   //!< Уровень ошибок
extern const LoggerMessageLevel LFATAL;   //!< Уровень критических ошибок
extern const LoggerMessageLevel LSTAT;    //!< Уровень статистической информации

} /* namespace Logger */
} /* namespace Tools */
#endif /* LOGGERMESSAGELEVEL_H_ */
