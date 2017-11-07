// POSIX
#include <signal.h>
#include <string.h>

// C++
#include <fstream>
#include <stdexcept>

// Boost
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_array.hpp>

// Pion
#include <pion/services/FileService.hpp>

#include "Tools/Configuration/XmlParser.h"
#include "Tools/Logger/Logger.h"
#include "Tools/MiscUtils/ShutdownManager.h"
#include "Tools/WebServer/RedirectService.h"

// This
#include "Tools/WebSvcApp/WebSvcApp.h"

namespace po = boost::program_options;

using namespace Tools::Configuration;

namespace Tools
{
namespace WebSvcApp
{

/**
 * Обработчик системных сигналов завершения
 */
void handleShutdownSignal(int)
{
    ShutdownManager::get_mutable_instance().shutdown(); // Завершить ожидание
}

void logBackTrace(int sig)
{
  Tools::Logger::Logger &logger = Tools::Logger::Logger::getInstance();
    std::cout << "Shutting down by signal " << sig << std::endl;
    logger.fatal(std::string("Shutting down by signal ") + boost::lexical_cast<std::string>(sig));

    //void *array[20];
    //size_t size;
    //char **strings = NULL;

    //// get void*'s for all entries on the stack
    //size = backtrace(array, 20);
    //strings = backtrace_symbols(array, size);

    //for (size_t i = 0; i < size; i++)
    //{
    //    std::cerr << strings[i] << std::endl;
    //    logger.fatal(strings[i]);
    //}

    //free( strings );
    exit(255);
}

class StatStub : public Tools::WebServer::IStat
{
public:
    virtual ~StatStub() {}

    virtual void registerParameter(const std::string &) {}
    virtual void set(const std::string &, const long) {}
    virtual void increment(const std::string &, const long) {}
    virtual void registerParametersProvider(const std::string &,
    		                                const ParametersProvider &){};
    virtual void unregisterParametersProvider(const std::string &){};
};

//--------------------------------------------------------------------------------------------------
WebSvcApp::WebSvcApp(const std::string &project, const std::string &version, const std::string &revision):
        m_project(project), m_version(version), m_revision(revision)
{
}

//--------------------------------------------------------------------------------------------------
WebSvcApp::~WebSvcApp()
{
}

//--------------------------------------------------------------------------------------------------
int WebSvcApp::run(int argc, char **argv)
{
    // Чтение опций командной строки
    try
    {
        if (!processCommandLine(argc, argv))
        {
            return EXIT_SUCCESS;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Invalid command line options: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Чтение конфигурации
    Tools::Configuration::Configuration conf;
    try
    {
        conf = Parsers::Xml::readXml(getConfigFileName()).branch("document");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Configuration error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return run(conf);
}

int WebSvcApp::run(Tools::Configuration::Configuration &conf)
{
    Tools::Logger::Logger &logger = getLogger();

    m_config = conf;

    try {
        // Конфигурируем логгер
        if (!getConf().exists("run.logger")) 
        {
            throw std::runtime_error("Missing logger configuration section (run.logger)");
        }
        logger.configure(getConf().branch("run.logger"));

        logger.info("");
        logger.info("");
        logger.info("Starting service " + getProject() + " " + getVersion());

        // Настройка HTTP-сервера
        ConfigurationView httpConfig = getConf().branch("run.httpserver");

        logger.info("Configuring WebServer...");

        boost::shared_ptr<Tools::WebServer::WebServer> webServerPtr;
        {
            const std::string httpHost = httpConfig.get<std::string>("host", std::string("0.0.0.0"));
            const int httpPort = httpConfig.get<int>("port", 80);
            const std::size_t httpTimeout = httpConfig.get<std::size_t>("timeout", 1u);
            const std::size_t httpConnectionLimit = httpConfig.get<int>("connectionlimit", 100u);
            const std::size_t httpThreads = httpConfig.get<std::size_t>("httpthreads", 8u);
            const std::size_t workerThreads = httpConfig.get<std::size_t>("workerthreads", 16u);

            webServerPtr.reset(new Tools::WebServer::WebServer(httpHost, httpPort, httpThreads, httpTimeout, httpConnectionLimit, workerThreads));

            logger.info() << "Host=" << httpHost << ":" << httpPort << ", "
                << "httpThreads=" << httpThreads << ", "
                << "workerThreads=" << workerThreads << ", "
                << "httpConnectionLimit=" << httpConnectionLimit << ", "
                << "httpTimeout=" << httpTimeout << ".";
        }

        // настроить статистику
        Tools::WebServer::IStatPtr statPtr;
        {
            const std::string statServiceName = httpConfig.get<std::string>("statservicename", m_project);
            const std::string statServiceResource = httpConfig.get<std::string>("statservice", std::string(""));

            if (!statServiceName.empty() && !statServiceResource.empty())
            {
                webServerPtr->enableStatService(statServiceName,
                    "/" + statServiceResource,
                    m_version,
                    m_revision);

                statPtr = webServerPtr->getStatService();
                logger.info() << "Stat service enabled (statServiceName=" << statServiceName
                    << ", statServiceResource=/" << statServiceResource << ").";
            }
            else 
            {
                statPtr.reset(new StatStub());
                logger.info("Stat service disabled (to enable set \"run.httpserver.statservice\" and \"run.httpserver.statservicename\").");
            }
        }

        logger.info("WebServer configured.");

        // Инициализация
        {
            logger.info("Initializing web-services...");
            WebServiceRegistrar registrar(webServerPtr);
            initialize(registrar, statPtr);

            if (!registrar.hasServices()) 
            {
                throw std::runtime_error("No web services registered.");
            }

            logger.info("Web-services initialized.");
        }

        if (getConf().exists("serviceconfig.plugins"))
        {
            logger.info("Initializing plugin web-services...");
            
            ConfigurationView pluginsConf = getConf().branch("serviceconfig.plugins");
            BOOST_FOREACH(const ConfigurationView &pluginConf, pluginsConf.getRangeOf("plugin"))
            {
                const std::string pluginName = pluginConf.getAttr("", "name");
                const std::string resource = pluginConf.get<std::string>("resource", std::string("/"));
                Tools::WebServer::PluginServiceOptions opts;
                if (pluginConf.exists("options"))
                {
                    BOOST_FOREACH(const ConfigurationView &optConf, pluginConf.branch("options").getChildren())
                    {
                        opts[*optConf.pathLocation().getElements().crbegin()] = optConf.get("");
                    }
                }

                if (pluginName == "fileservice")
                {
                    webServerPtr->addPluginService(resource, new pion::plugins::FileService(), opts);
                }
            }
            logger.info("Plugin web-services initialized.");
        }

        if (getConf().exists("serviceconfig.redirects"))
        {
            logger.info("Initializing redirects...");

            ConfigurationView redirectsConf = getConf().branch("serviceconfig.redirects");
            BOOST_FOREACH(const ConfigurationView &redirectConf, redirectsConf.getRangeOf("redirect"))
            {
                const std::string from = redirectConf.getAttr("", "from");
                const std::string to = redirectConf.getAttr("", "to");
                if (redirectConf.hasAttr("", "with_status"))
                {
                    const int httpStatus = redirectConf.getAttr<int>("", "with_status");
                    Tools::WebServer::IWebServicePtr redirectServicePtr(new Tools::WebServer::RedirectService(from, to, httpStatus));
                    webServerPtr->addService(from, redirectServicePtr);
                }
                else
                {
                    webServerPtr->addRedirect(from, to);
                }

            }
            logger.info("Redirects initialized.");
        }

        // Запуск HTTP-сервера
        logger.info("Starting WebServer...");
        webServerPtr->start();
        logger.info("WebServer started and ready to accept connections.");

        //------ Установливаем обработчик на системные сигналы завершения ------
        signal(SIGINT, handleShutdownSignal);
        signal(SIGBREAK, handleShutdownSignal);
        signal(SIGTERM, handleShutdownSignal);
        signal(SIGSEGV, logBackTrace);

        //-------------- Ожидаем сигнала завершения или ошибки -----------------
        ShutdownManager::get_mutable_instance().wait();

        // Остановка сервера
        logger.info("Stopping WebServer...");
        webServerPtr->stop();
        webServerPtr.reset();
        logger.info("WebServer stopped.");

        // Очистка
        logger.info("Cleaning up...");
        cleanup();
        logger.info("Done.");

        logger.info("Exit.");

        return EXIT_SUCCESS;
    } 
    catch (const std::exception &e) 
    {
        std::cerr << "FATAL: " << e.what() << std::endl;
        logger.fatal(std::string(e.what()));
    } 
    catch (...) 
    {
        std::cerr << "FATAL: unknown error" << std::endl;
        logger.fatal(std::string("Unknown error"));
    }

    return EXIT_FAILURE;
}
//--------------------------------------------------------------------------------------------------
bool WebSvcApp::processCommandLine(int argc, char **argv)
{
    po::options_description optionsDescription("Allowed options");
    optionsDescription.add_options()
            ("help,h", "Display the program usage and exit")
            ("version,v", "Display the program version and exit")
            ("config,c", po::value<std::string>(&m_configFileName)->required(), "Path to configuration file (required)");

    // Добавить пользовательские опции
    registerOptions(optionsDescription);

    po::variables_map options;
    po::store(po::command_line_parser(argc, argv).options(optionsDescription).run(), options);

    if (options.count("help"))
    {
        std::cerr << "Usage: " << argv[0] << " --config=configuration-file" << std::endl << std::endl
                << optionsDescription << std::endl;
        return false;
    }
    else if (options.count("version"))
    {
        std::cout << getVersion() << std::endl;
        return false;
    }

    po::notify(options);
    return true;
}

//--------------------------------------------------------------------------------------------------
void WebSvcApp::registerOptions(boost::program_options::options_description &)
{
}

//--------------------------------------------------------------------------------------------------
void WebSvcApp::cleanup()
{
}

//--------------------------------------------------------------------------------------------------
const std::string &WebSvcApp::getProject() const
{
    return m_project;
}

//--------------------------------------------------------------------------------------------------
const std::string &WebSvcApp::getVersion() const
{
    return m_version;
}

//--------------------------------------------------------------------------------------------------
const std::string &WebSvcApp::getConfigFileName() const
{
    return m_configFileName;
}

//--------------------------------------------------------------------------------------------------
Tools::Configuration::ConfigurationView WebSvcApp::getConf() const
{
    return m_config;
}

//--------------------------------------------------------------------------------------------------
Tools::Logger::Logger &WebSvcApp::getLogger()
{
    return Tools::Logger::Logger::getInstance();
}

//--------------------------------------------------------------------------------------------------
std::string WebSvcApp::getHostName()
{
    unsigned bufSize = 1024u;
    boost::scoped_array<char> buf(new char[bufSize]);

    while (gethostname(buf.get(), bufSize) != 0)
    {
        if (errno != ENAMETOOLONG)
        {
            throw std::runtime_error(std::string("gethostname() error: ") + strerror(errno));
        }
        bufSize *= 2u;
        buf.reset(new char[bufSize]);
    }

    return std::string(buf.get());
}

} // namespace WebSvcApp
} // namespace Tools
