#ifndef TOOLS_MISCUTILS_SHUTDOWNMANAGER_H_
#define TOOLS_MISCUTILS_SHUTDOWNMANAGER_H_

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/serialization/singleton.hpp>

/**
 * @brief Менеджер ожидания
 *
 * Класс осуществляет блокировку потока выполнения до вызова метода завершения
 *
 * Пример:
 * @code
 * ...
 * void handleShutdownSignal(int)
 * {
 *     ShutdownManager::get_mutable_instance().shutdown();
 * }
 * ...
 * signal(SIGINT, handleSignal);
 * ...
 * ShutdownManager::get_mutable_instance().wait(); // Ожидает системного сигнала завершения SIGINT
 * ...
 * @endcode
 *
 * Шаблон проектирования: <i>Singleton/Одиночка</i>
 *
 * <b>thread-safe</b>
 */
class ShutdownManager : public boost::serialization::singleton<ShutdownManager>
{
public:
    /**
     * Создает объект
     */
    ShutdownManager(void) : m_shutdownNow(false)
    {
    }

    /**
     * Завершает ожидание (снимет блокировку)
     */
    inline void shutdown(void)
    {
        boost::mutex::scoped_lock lock(m_mutex);
        m_shutdownNow = true;
        m_condition.notify_all();
    }

    /**
     * Инициирует ожидание (блокирует)
     * @warning метод блокирует поток выполнения до вызова @a shutdown
     */
    inline void wait(void)
    {
        boost::mutex::scoped_lock lock(m_mutex);
        while (!m_shutdownNow)
        {
            m_condition.wait(lock);
        }
    }

private:
    /** Завершить ожидание */
    bool m_shutdownNow;
    /** Мьютекс */
    boost::mutex m_mutex;
    /** Условная переменная ожидания */
    boost::condition m_condition;
};

#endif /* TOOLS_MISCUTILS_SHUTDOWNMANAGER_H_ */
