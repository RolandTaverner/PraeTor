#ifndef ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ASYNCHTTPCLIENT_H_
#define ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ASYNCHTTPCLIENT_H_

// C++
#include <list>
#include <string>

// Boost
#include <boost/asio/ip/tcp.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/future.hpp>
#include <boost/tuple/tuple.hpp>

// Pion
#include <pion/http/request.hpp>
#include <pion/scheduler.hpp>

// This
#include "Tools/AsyncHttpClient/IConnectionPool.h"
#include "Tools/AsyncHttpClient/IRequestManager.h"
#include "Tools/AsyncHttpClient/HandlerTypes.h"

namespace Tools
{
namespace AsyncHttpClient
{

typedef boost::tuple<pion::http::request_ptr, const boost::asio::ip::tcp::endpoint &, bool> RequestDesc;
typedef std::list<RequestDesc> MultiRequestDesc;

typedef boost::future<RequestResult> SingleRequestFuture;
typedef boost::future<RequestResults> MultiRequestFuture;

/**
 * @brief Класс для создания асинхронных HTTP-запросов
 *
 * По сути является фабрикой.
*/
class AsyncHttpClient : boost::noncopyable
{
private:
    /**
     * Конструктор по умолчанию
     */
    AsyncHttpClient();

public:

    /**
     * Конструктор
     * @param connPoolPtr - пул соединений
     * @param schedulerPtr - шедулер
     */
    AsyncHttpClient(IConnectionPoolPtr connPoolPtr,
                    const boost::shared_ptr<pion::scheduler> &schedulerPtr);

    /**
     * Деструктор
     */
    virtual ~AsyncHttpClient();

    /**
     * Создает объект, управляющий одним запросом
     * @param request - параметры запроса
     * @param finishedHandler - хэндлер завершения запроса
     * @param timeout - таймаут
     * @param startImmediately - флаг нмедленного запуска запроса
     * @return объект, управляющий запросом
     */
    IRequestManagerPtr createRequest(const RequestDesc &request,
                                     const SingleRequestHandler &finishedHandler,
                                     const boost::posix_time::time_duration &timeout,
                                     bool startImmediately = true);

    /**
     * Создает объект, управляющий выполнением группы запросов
     * @param request - параметры запросов (список)
     * @param finishedHandler - хэндлер завершения запроса
     * @param timeout - таймаут (общий для всех запросов)
     * @param startImmediately - флаг нмедленного запуска запросов
     * @return объект, управляющий запросами
     */
    IRequestManagerPtr createRequest(const MultiRequestDesc &request,
                                     const MultiRequestHandler &finishedHandler,
                                     const boost::posix_time::time_duration &timeout,
                                     bool startImmediately = true);

    /**
     * Создает boost::future<RequestResult>, связанный с запросом
     * @param request - параметры запроса
     * @param timeout - таймаут
     * @param tracerPtr - трассировщик
     * @return boost::future<RequestResult>
     */
    SingleRequestFuture createFuture(const RequestDesc &request,
                                     const boost::posix_time::time_duration &timeout);

    /**
     * Создает boost::future<RequestResults>, связанный с группой запросов
     * @param request - параметры запросов (список)
     * @param timeout - таймаут (общий для всех запросов)
     * @param tracerPtr - трассировщик
     * @return boost::future<RequestResults>
     */
    MultiRequestFuture createFuture(const MultiRequestDesc &request,
                                    const boost::posix_time::time_duration &timeout);

private:
    IConnectionPoolPtr m_connPoolPtr;
    boost::shared_ptr<pion::scheduler> m_schedulerPtr;
};

typedef boost::shared_ptr<AsyncHttpClient> AsyncHttpClientPtr;

} /* namespace AsyncHttpClient */
} /* namespace Tools */

#endif /* ASYNCHTTPCLIENT_INCLUDE_TOOLS_ASYNCHTTPCLIENT_ASYNCHTTPCLIENT_H_ */
