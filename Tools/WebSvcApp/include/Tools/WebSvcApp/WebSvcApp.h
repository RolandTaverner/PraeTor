#ifndef INCLUDE_SERVICESLIBS_WEBSVCAPP_WEBSVCAPP_H_
#define INCLUDE_SERVICESLIBS_WEBSVCAPP_WEBSVCAPP_H_

// C++
#include <string>

// Boost
#include <boost/noncopyable.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/shared_ptr.hpp>

// Tools
#include "Tools/Configuration/Configuration.h"
#include "Tools/Logger/Logger.h"
#include "Tools/WebServer/WebServer.h"

namespace Tools
{
namespace WebSvcApp
{

/**
 * Класс для реализации веб-сервисов
 *
 */
class WebSvcApp : boost::noncopyable
{
public:
    class WebServiceRegistrar
    {
    public:
        explicit WebServiceRegistrar(boost::shared_ptr<Tools::WebServer::WebServer> webServerPtr):
        m_webServerPtr(webServerPtr), m_addedServices(false)
        {
        }

        void registerService(const std::string &resource, Tools::WebServer::IWebServicePtr servicePtr)
        {
            m_webServerPtr->addService(resource, servicePtr);
            m_addedServices = true;
        }

        bool hasServices() const
        {
            return m_addedServices;
        }

    private:
        boost::shared_ptr<Tools::WebServer::WebServer> m_webServerPtr;
        bool m_addedServices;
    };

public:
    WebSvcApp(const std::string &project, const std::string &version, const std::string &revision);
    virtual ~WebSvcApp();

    /**
     * Запускает сервис
     * @param argc количество параметров
     * @param argv параметры
     * @return код возврата
     */
    int run(int argc, char **argv);

    /**
    * Запускает сервис
    * @param argc количество параметров
    * @param argv параметры
    * @return код возврата
    */
    int run(Tools::Configuration::Configuration &conf);

    /**
     * Возвращает название проекта
     * @return название проекта
     */
    const std::string &getProject() const;

    /**
     * Возвращает версию проекта
     * @return версия проекта
     */
    const std::string &getVersion() const;

    /**
     * Возвращает путь к конфигурационному файлу
     * @return путь к конфигурационному файлу
     */
    const std::string &getConfigFileName() const;

    /**
     * Возвращает конфигурацию
     * @return конфигурация
     */
    Tools::Configuration::ConfigurationView getConf() const;

    /**
     * Возвращает логгер
     * @return логгер
     */
    Tools::Logger::Logger &getLogger();

    /**
     * Возвращает имя хоста
     * @return имя хоста
     */
    static std::string getHostName();

protected:
    /**
     * Добавляет пользовательские параметры командной строки
     * @param options описание параметров командной стоки
     */
    virtual void registerOptions(boost::program_options::options_description &options);

    /**
     * Выполняет инициализацию веб-сервисов
     * @param webServiceRegistrar объект для регистрации сервисов
     * @param statPtr объект для настойки параметров статистики
     */
    virtual void initialize(WebServiceRegistrar &webServiceRegistrar, Tools::WebServer::IStatPtr statPtr) = 0;

    /**
     * Выполняет очистку перед остановкой сервиса
     */
    virtual void cleanup();

private:
    /**
     * Производит обработку параметров командной строки
     * @param argc - количество параметров
     * @param argv - параметры
     * @return Возвращает false если необходимо завершить выполнение программы, иначе - true
     * @throw program_options::error - если произошла ошибка обработки
     */
    bool processCommandLine(int argc, char **argv);

private:
    std::string m_project;
    std::string m_version;
    std::string m_revision;
    std::string m_configFileName;
    Tools::Configuration::Configuration m_config;
};

} // namespace WebSvcApp
} // namespace Tools

#endif /* INCLUDE_SERVICESLIBS_WEBSVCAPP_WEBSVCAPP_H_ */
