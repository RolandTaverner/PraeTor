#ifndef SCHEDULER_H_
#define SCHEDULER_H_

// C++
#include <map>
#include <stddef.h>

// BOOST
#include <boost/asio/io_service.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "Tools/WebServer/IScheduler.h"

namespace Tools
{
namespace WebServer
{

class Scheduler : public ITimerScheduler, boost::noncopyable
{
public:
    Scheduler(const std::size_t minThreads, const std::size_t maxThreads);
    virtual ~Scheduler();

    void start();
    void stop();

    // IScheduler
    virtual void execute(SchedulerHandler handler);
    // ITimerScheduler
    virtual DeadlineTimerPtr executeOnTimer(SchedulerTimerHandler handler,
                                    const boost::posix_time::time_duration &timeDuration);

    bool isRunning() const;

public:
    //types

    class TerminateException : public std::exception
    {
    public:
        TerminateException()
        {
            m_message = "Terminate thread request";
        }

        virtual ~TerminateException() 
        {
        }

        virtual const char* what() const 
        {
            return m_message.c_str();
        }

    private:
        std::string m_message;
    };

    enum ThreadState
    {
        TS_IDLE, TS_BUSY, TS_ZOMBIE
    };

    class ThreadInfo
    {
    public:

        ThreadInfo(boost::shared_ptr<boost::thread> threadPtr, ThreadState state) :
                        m_threadPtr(threadPtr),
                        m_threadState(state)
        {
        }

        ThreadInfo(const ThreadInfo &rhs)
        {
            *this = rhs;
        }

        ~ThreadInfo()
        {
        }

        ThreadInfo &operator=(const ThreadInfo &rhs)
        {
            if (this != &rhs)
            {
                m_threadPtr = rhs.m_threadPtr;
                m_threadState = rhs.m_threadState;
            }
            return *this;
        }

        void startHandler() 
        {
            setState(TS_BUSY);
        }

        void finishHandler() 
        {
            setState(TS_IDLE);
        }

        boost::shared_ptr<boost::thread> getThreadPtr()
        {
            return m_threadPtr;
        }

        void setThreadPtr(boost::shared_ptr<boost::thread> threadPtr)
        {
            m_threadPtr = threadPtr;
        }

        ThreadState getState() 
        {
            return m_threadState;
        }

        void setState(const ThreadState state) 
        {
            m_threadState = state;
        }

    private:
        boost::shared_ptr<boost::thread> m_threadPtr;
        ThreadState m_threadState;
    };

private:
    //types

    class BaseHandlerWrapper
    {
    public:
        explicit BaseHandlerWrapper(Scheduler *pExecutor) :
                m_pExecutor(pExecutor)
        {
        }

        BaseHandlerWrapper(const BaseHandlerWrapper &rhs)
        {
            *this = rhs;
        }

        BaseHandlerWrapper &operator=(const BaseHandlerWrapper &rhs)
        {
            if (this != &rhs)
            {
                m_pExecutor = rhs.m_pExecutor;
            }
            return *this;
        }

        virtual ~BaseHandlerWrapper()
        {
        }

    private:

        void beforeExecute()
        {
            m_pExecutor->getThreadInfo()->startHandler();
        }

        void afterExecute()
        {
            m_pExecutor->getThreadInfo()->finishHandler();
            m_pExecutor->onFinishExec();
        }

        class ExecutionLock
        {
        public:
            ExecutionLock(BaseHandlerWrapper *pBaseHandlerWrapper) :
                    m_pBaseHandlerWrapper(pBaseHandlerWrapper)
            {
                m_pBaseHandlerWrapper->beforeExecute();
            }

            ~ExecutionLock()
            {
                m_pBaseHandlerWrapper->afterExecute();
            }
        private:
            BaseHandlerWrapper *m_pBaseHandlerWrapper;
        };
        friend class ExecutionLock;

        Scheduler *m_pExecutor;

    public:
        void executeHandler(SchedulerHandler handler)
        {
            ExecutionLock lock(this);
            handler();
        }
    };

    class HandlerWrapper : public BaseHandlerWrapper
    {
    public:
        HandlerWrapper(Scheduler *pExecutor, SchedulerHandler handler);
        HandlerWrapper(const HandlerWrapper &rhs);
        HandlerWrapper &operator=(const HandlerWrapper &rhs);
        virtual ~HandlerWrapper()
        {
        }

        void operator()();

    private:
        SchedulerHandler m_handler;
    };

    class TimerHandlerWrapper : public BaseHandlerWrapper
    {
    public:
        TimerHandlerWrapper(Scheduler *pExecutor,
                            SchedulerTimerHandler handler);
        TimerHandlerWrapper(const TimerHandlerWrapper &rhs);
        TimerHandlerWrapper &operator=(const TimerHandlerWrapper &rhs);
        virtual ~TimerHandlerWrapper()
        {
        }

        void operator()(const boost::system::error_code &error);

    private:
        SchedulerTimerHandler m_handler;
    };

private:
    //members

    void setRunning(bool running);
    void addThread();
    void killThread();
    void deleteThread(const boost::thread::id &id);
    void checkThreads();
    void onFinishExec();
    void iosRunner(ThreadInfo *pThreadInfo);
    void markThreadAsZombie();
    boost::thread_specific_ptr<ThreadInfo> &getThreadInfo();

    std::size_t m_minThreads;
    std::size_t m_maxThreads;
    boost::asio::io_service m_ioService;
    boost::scoped_ptr<boost::asio::io_service::work> m_pWork;

    typedef std::map<boost::thread::id, boost::shared_ptr<ThreadInfo> > Threads;
    Threads m_threads;
    volatile bool m_isRunning;

    boost::mutex m_mutex;
    volatile std::size_t m_pendingRequests;
    volatile std::size_t m_threadsCount;

    boost::thread_specific_ptr<ThreadInfo> m_threadInfo;
};

typedef boost::shared_ptr<Scheduler> SchedulerPtr;

} /* namespace WebServer */
} /* namespace Tools */

#endif /* SCHEDULER_H_ */
