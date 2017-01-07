#ifndef ISCHEDULER_H_
#define ISCHEDULER_H_

// BOOST
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace Tools
{
namespace WebServer
{

typedef boost::function<void()> SchedulerHandler;
typedef boost::function<void(const boost::system::error_code &)> SchedulerTimerHandler;
typedef boost::shared_ptr<boost::asio::deadline_timer> DeadlineTimerPtr;

class IScheduler
{
public:
    virtual ~IScheduler()
    {}

    virtual void execute(SchedulerHandler handler) = 0;
};

typedef boost::shared_ptr<IScheduler> ISchedulerPtr;

class ITimerScheduler : public IScheduler
{
public:
    virtual ~ITimerScheduler()
    {}

    virtual DeadlineTimerPtr executeOnTimer(SchedulerTimerHandler handler,
                                            const boost::posix_time::time_duration &timeDuration) = 0;
};

typedef boost::shared_ptr<ITimerScheduler> ITimerSchedulerPtr;

} /* namespace WebServer */
} /* namespace Tools */

#endif /* ISCHEDULER_H_ */
