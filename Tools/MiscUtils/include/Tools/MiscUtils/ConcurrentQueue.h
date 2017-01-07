#ifndef TOOLS_MISCUTILS_CONCURRENTQUEUE_H_
#define TOOLS_MISCUTILS_CONCURRENTQUEUE_H_

#include <queue>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace Tools
{
namespace MiscUtils
{

/**
 * @brief Многопоточная очередь
 * @tparam T - тип данных очереди
 *
 * Класс реализует многопоточную очередь данных.
 * Для межпоточной синхронизации используется boost::mutex, boost::condition
 *
 * <b>thread-safe</b>
 */
template<typename T>
class ConcurrentQueue
{
public:
    /**
     * Помещает данные в очередь
     * @param data
     */
    void push(T const& data)
    {
        boost::mutex::scoped_lock lock(m_mutex);

        m_queue.push(data);
        lock.unlock();
        m_condition.notify_one();
    }

    /** @return true - если очерьедь пустая, иначе false */
    bool isEmpty() const
    {
        boost::mutex::scoped_lock lock(m_mutex);

        return m_queue.empty();
    }

    /** @return количество элементов в очереди */
    size_t getSize() const
    {
        return m_queue.size();
    }

    /**
     * Извлекает данные из очереди
     * @param value - извлекаемые данные
     * @return true - если данные извлеклись, иначе false
     */
    bool tryPop(T& value)
    {
        boost::mutex::scoped_lock lock(m_mutex);

        if (m_queue.empty())
        {
            return false;
        }

        value = m_queue.front();
        m_queue.pop();

        return true;
    }

    /**
     * Ожидает данные и извлекает их из очереди
     * @param value - извлекаемые данные
     */
    void waitAndPop(T& value)
    {
        boost::mutex::scoped_lock lock(m_mutex);

        while (m_queue.empty())
        {
            m_condition.wait(lock);
        }

        value = m_queue.front();
        m_queue.pop();
    }

private:
    /** Очередь данных */
    std::queue<T> m_queue;
    /** Мьютекс */
    mutable boost::mutex m_mutex;
    /** Условная переменная ожидания */
    boost::condition m_condition;
};

} /* namespace MiscUtils */
} /* namespace Tools */

#endif /* CONCURRENTQUEUE_H_ */
