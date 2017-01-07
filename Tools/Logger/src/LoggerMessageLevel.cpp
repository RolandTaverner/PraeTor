#include "Tools/Logger/LoggerMessageLevel.h"

namespace Tools
{
namespace Logger
{

//--------------------------------------------------------------------------------------------------
LoggerMessageLevel::LoggerMessageLevel(const std::string &name,
                                       unsigned int maskLevel)  :
        m_name(name),
        m_maskLevel(maskLevel)
{
}

//--------------------------------------------------------------------------------------------------
const std::string &LoggerMessageLevel::getName() const 
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
unsigned int LoggerMessageLevel::getMask() const 
{
    return m_maskLevel;
}

//--------------------------------------------------------------------------------------------------
bool LoggerMessageLevel::operator==(const LoggerMessageLevel &right) const
{
    return m_name == right.m_name;
}

//--------------------------------------------------------------------------------------------------
bool LoggerMessageLevel::operator!=(const LoggerMessageLevel &right) const
{
    return m_name != right.m_name;
}

/** Уровни сообщений, определенные по умолчанию */
const LoggerMessageLevel LEMPTY("", 0);                  //!< Пустой уровень
const LoggerMessageLevel LDEBUG("DEBUG", L_DEBUG);       //!< Отладочный уровень
const LoggerMessageLevel LINFO("INFO", L_INFO);          //!< Информационный уровень
const LoggerMessageLevel LTRACE("TRACE", L_TRACE);       //!< Трассировочный уровень
const LoggerMessageLevel LWARNING("WARNING", L_WARNING); //!< Уровень предупреждений
const LoggerMessageLevel LERROR("ERROR", L_ERROR);       //!< Уровень ошибок
const LoggerMessageLevel LFATAL("FATAL", L_FATAL);       //!< Уровень критических ошибок
const LoggerMessageLevel LSTAT("STAT", L_STAT);          //!< Уровень статистической информации

} /* namespace Logger */
} /* namespace Tools */
