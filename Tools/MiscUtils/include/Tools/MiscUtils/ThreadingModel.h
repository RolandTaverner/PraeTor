#ifndef TOOLS_MISCUTILS_THREADINGMODEL_H_
#define TOOLS_MISCUTILS_THREADINGMODEL_H_

#include <boost/thread/mutex.hpp>

namespace Tools
{
namespace MiscUtils
{

/**
 * @brief Блокировщик
 *
 * Класс для автоматической "scoped" блокировки/разблокировки
 *
 * @tparam тип объекта блокировки
 */
template<class T>
class Locker
{
public:
    /**
     * Создает объект, производит блокировку
     * @param object - объект блокировки
     */
    Locker(T &object)  : m_object(object)
    {
        m_object.lock();
    }
    /**
     * Уничтожает объект, производит разблокировку
     */
    ~Locker() 
    {
        m_object.unlock();
    }

private:
    /** Объект блокировки */
    T &m_object;
};

/**
 * @brief Модель блокировки для однопоточной модели
 */
struct SingleThreading
{
    void lock() 
    {
    }

    void unlock() 
    {
    }
};

/**
 * @brief Модель блокировки для многопоточной модели
 */
typedef boost::mutex MultiThreading;

} // namespace MiscUtils
} // namespace Tools

#endif /* TOOLS_MISCUTILS_THREADINGMODEL_H_ */
