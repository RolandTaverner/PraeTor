// BOOST
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/tss.hpp>

// THIS
#include "Tools/WebServer/Scheduler.h"

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
static void deleteThreadInfoStub(Scheduler::ThreadInfo *)
{
}

//--------------------------------------------------------------------------------------------------
Scheduler::Scheduler(const std::size_t minThreads, const std::size_t maxThreads) :
                m_minThreads(minThreads),
                m_maxThreads(maxThreads),
                m_isRunning(false),
                m_pendingRequests(0u),
                m_threadsCount(0u),
                m_threadInfo(&deleteThreadInfoStub)
{
}

//--------------------------------------------------------------------------------------------------
Scheduler::~Scheduler()
{
    stop();
}

//--------------------------------------------------------------------------------------------------
void Scheduler::start()
{
    if (!isRunning())
    {
        m_pWork.reset(new boost::asio::io_service::work(m_ioService));
        for (std::size_t i = 0u; i < m_minThreads; ++i)
        {
            addThread();
        }
        setRunning(true);
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::stop()
{
    if (isRunning())
    {
        m_pWork.reset();
        m_ioService.stop();
        for (Threads::iterator i = m_threads.begin(); i != m_threads.end(); ++i)
        {
            i->second->getThreadPtr()->join();
        }
        m_threads.clear();
        m_ioService.reset();
        m_pendingRequests = 0u;
        setRunning(false);
    }
}

//--------------------------------------------------------------------------------------------------
bool Scheduler::isRunning() const
{
    return m_isRunning;
}

//--------------------------------------------------------------------------------------------------
void Scheduler::setRunning(bool running)
{
    m_isRunning = running;
}

//--------------------------------------------------------------------------------------------------
void Scheduler::execute(SchedulerHandler handler)
{
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        ++m_pendingRequests;
        checkThreads();
    }

    HandlerWrapper hw(this, handler);
    m_ioService.post(hw);
}

//--------------------------------------------------------------------------------------------------
DeadlineTimerPtr Scheduler::executeOnTimer(SchedulerTimerHandler handler,
                                           const boost::posix_time::time_duration &timeDuration)
{
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        ++m_pendingRequests;
        checkThreads();
    }

    TimerHandlerWrapper hw(this, handler);

    boost::shared_ptr<boost::asio::deadline_timer> pTimer(new boost::asio::deadline_timer(m_ioService,
                                                                                          timeDuration));
    pTimer->async_wait(hw);
    return pTimer;
}

//--------------------------------------------------------------------------------------------------
void Scheduler::iosRunner(ThreadInfo *pThreadInfo)
{
    const boost::thread::id id = boost::this_thread::get_id();

    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        m_threadInfo.reset(pThreadInfo);
    }

    try
    {
        m_ioService.run();
    }
    catch (const boost::system::system_error &)
    {
        markThreadAsZombie();
        execute(boost::bind(&Scheduler::deleteThread, this, id));
    }
    catch (const TerminateException &)
    {
        markThreadAsZombie();
        execute(boost::bind(&Scheduler::deleteThread, this, id));
    }
    catch (const std::exception &)
    {
        markThreadAsZombie();
        execute(boost::bind(&Scheduler::deleteThread, this, id));
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::markThreadAsZombie()
{
    getThreadInfo()->setState(TS_ZOMBIE);
}

//--------------------------------------------------------------------------------------------------
void threadStopper()
{
    throw Scheduler::TerminateException();
}

//--------------------------------------------------------------------------------------------------
void Scheduler::addThread()
{
    if (m_threadsCount < m_maxThreads)
    {
        boost::shared_ptr<boost::thread> threadPtr;
        boost::shared_ptr<ThreadInfo> threadInfoPtr(new ThreadInfo(threadPtr, TS_IDLE));
        threadPtr.reset(new boost::thread(boost::bind(&Scheduler::iosRunner,
                                                      boost::ref(*this),
                                                      threadInfoPtr.get())));
        threadInfoPtr->setThreadPtr(threadPtr);
        m_threads.insert(std::make_pair(threadPtr->get_id(), threadInfoPtr));
        ++m_threadsCount;
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::killThread()
{
    if (m_threadsCount > m_minThreads)
    {
        --m_threadsCount;
        m_ioService.post(threadStopper);
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::deleteThread(const boost::thread::id &id)
{
    boost::lock_guard<boost::mutex> lock(m_mutex);
    Threads::iterator it = m_threads.find(id);

    BOOST_ASSERT(it != m_threads.end());

    if (it != m_threads.end())
    {
        it->second->getThreadPtr()->join();
        m_threads.erase(it);
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::checkThreads()
{
    int threadsDelta = 0;
    {
        const double threads = m_threadsCount;
        const double requests = m_pendingRequests;
        const double ratio = requests / threads;
        if (ratio > 1.5)
        {
            threadsDelta = threads * (ratio - 1.0);
        }
        else if (ratio < 0.5)
        {
            threadsDelta = -1 * threads * (1.0 - ratio);
        }
    }

    if (threadsDelta > 0)
    {
        for (int i = 0; i < threadsDelta; ++i)
        {
            addThread();
        }
    }
    else if (threadsDelta < 0)
    {
        for (int i = 0; i < -1 * threadsDelta; ++i)
        {
            killThread();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Scheduler::onFinishExec()
{
#ifdef DEBUG
    std::size_t busyThreads = 0u;
    std::size_t threadsCount = 0u;
#endif

    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        --m_pendingRequests;

        checkThreads();

#ifdef DEBUG
        for(Threads::const_iterator it = m_threads.begin(); it != m_threads.end(); ++it)
        {
            if (it->second->getState() == TS_BUSY)
            {
                ++busyThreads;
            }

            if (it->second->getState() != TS_ZOMBIE)
            {
                ++threadsCount;
            }
        }
#endif
    }

#ifdef DEBUG
    std::size_t thisPtr = reinterpret_cast<std::size_t>(this);
    const std::string thisId = boost::lexical_cast<std::string>(thisPtr);
#endif
}

//--------------------------------------------------------------------------------------------------
boost::thread_specific_ptr<Scheduler::ThreadInfo> &Scheduler::getThreadInfo()
{
    return m_threadInfo;
}

//--------------------------------------------------------------------------------------------------
Scheduler::HandlerWrapper::HandlerWrapper(Scheduler *pExecutor, SchedulerHandler handler) :
        Scheduler::BaseHandlerWrapper(pExecutor), m_handler(handler)
{
}

//--------------------------------------------------------------------------------------------------
Scheduler::HandlerWrapper::HandlerWrapper(const Scheduler::HandlerWrapper &rhs) :
        Scheduler::BaseHandlerWrapper(rhs)
{
    *this = rhs;
}

//--------------------------------------------------------------------------------------------------
Scheduler::HandlerWrapper &Scheduler::HandlerWrapper::operator=(const Scheduler::HandlerWrapper &rhs)
{
    if (this != &rhs)
    {
        this->BaseHandlerWrapper::operator=(rhs);
        m_handler = rhs.m_handler;
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
void Scheduler::HandlerWrapper::operator()()
{
    executeHandler(m_handler);
}

//--------------------------------------------------------------------------------------------------
Scheduler::TimerHandlerWrapper::TimerHandlerWrapper(Scheduler *pExecutor,
                                                    SchedulerTimerHandler handler) :
        Scheduler::BaseHandlerWrapper(pExecutor), m_handler(handler)
{
}

//--------------------------------------------------------------------------------------------------
Scheduler::TimerHandlerWrapper::TimerHandlerWrapper(const Scheduler::TimerHandlerWrapper &rhs) :
        Scheduler::BaseHandlerWrapper(rhs)
{
    *this = rhs;
}

//--------------------------------------------------------------------------------------------------
Scheduler::TimerHandlerWrapper &Scheduler::TimerHandlerWrapper::operator=(const Scheduler::TimerHandlerWrapper &rhs)
{
    if (this != &rhs)
    {
        this->BaseHandlerWrapper::operator=(rhs);
        m_handler = rhs.m_handler;
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
void Scheduler::TimerHandlerWrapper::operator()(const boost::system::error_code &error)
{
    executeHandler(SchedulerHandler(boost::bind(m_handler, error)));
}

} /* namespace WebServer */
} /* namespace Tools */
