#ifndef TOOLS_CONFIGURATION_CONFIGURATION_H_
#define TOOLS_CONFIGURATION_CONFIGURATION_H_

// C++
#include <list>

// Boost
#include <boost/lexical_cast.hpp>

#include "Tools/Configuration/ConfigurationBase.h"
#include "Tools/Configuration/ConfigurationView.h"
#include "Tools/Configuration/Path.h"
#include "Tools/Configuration/Ranges.h"
#include "Tools/Configuration/RealPath.h"

namespace Tools
{
namespace Configuration
{

/**
 * @brief Класс для работы с конфигурацией приложения
 *
 * Класс управляет выводом сообщений в журнал
 *
 * Пример использования:
 * Допустим, есть следующий файл конфигурации config.xml:
 * @pre <?xml version="1.0" encoding="utf-8"?>
 * @pre <config>
 * @pre     <run>1</run>
 * @pre     <servers>
 * @pre         <server>
 * @pre             <name>server-0</name>
 * @pre             <id>0</id>
 * @pre         </server>
 * @pre         <server>
 * @pre             <name>server-1</name>
 * @pre             <id>1</id>
 * @pre         </server>
 * @pre         <server>
 * @pre             <name>server-2</name>
 * @pre             <id>2</id>
 * @pre         </server>
 * @pre     </servers>
 * @pre </config>
 *
 * Пример работы с конфигурацией:
 *
 * @code
 * #include "Tools/Configuration/Configuration.h"
 * #include "Tools/Configuration/XmlParser.h"
 * using namespace Tools::Configuration;
 *
 * void processConfig()
 * {
 * Configuration root = Parsers::Xml::readXml("config.xml");
 *
 * Configuration config = root.branch("config");
 * bool run = config.get<bool>("run"); // == root.get<bool>("config.run");
 *
 * std::list<Configuration> serverConfigs = config.getListOf("servers.server");
 *
 * for(std::list<Configuration>::const_iterator i = serverConfigs.begin();
 *             i != serverConfigs.end();
 *             ++i)
 *     {
 *         Configuration serverConfig = *i;
 *         const std::string serverName = serverConfig.get("name");
 *         const int serverId = serverConfig.get<int>("id");
 *         // do some work with server
 *     }
 * }
 * @endcode
 */

class Configuration : public ConfigurationView, virtual public ConfigurationBase
{
public:
    /**
     * Конструктор по умолчанию
     */
    Configuration();

    /**
     * Конструктор копирования
     */
    Configuration(const Configuration &rhs);

    explicit Configuration(const ConfigurationBase &confBase);

    /**
     * Деструктор.
     * Так как @ref Configuration является представлением (view), хранилище данных
     * уничтожается только при уничтожении последнего объекта @ref Configuration,
     * использующего хранилище.
     */
    virtual ~Configuration();

    /**
     * Оператор присваивания
     * @param rhs - другой объект типа @ref Configuration
     * @return ссылку на себя
     */
    Configuration &operator=(const Configuration &rhs);

    /**
     * Устанавливает значение атрибута параметра конфигурации
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param attrName - имя атрибута
     * @param value - значение параметра конфигурации
     */
    template<typename T> void setAttr(const std::string &name, const std::string &attrName, const T &value)
    {
        setAttr(name, attrName, boost::lexical_cast<std::string>(value));
    }

    /**
     * Устанавливает значение атрибута параметра конфигурации
     * @param path - путь к параметру конфигурации
     * @param attrName - имя атрибута
     * @param value - значение параметра конфигурации
     */
    template<typename T> void setAttr(const Path &path, const std::string &attrName, const T &value)
    {
        setAttr(path, attrName, boost::lexical_cast<std::string>(value));
    }

    /**
     * Устанавливает значение атрибута параметра конфигурации
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param attrName - имя атрибута
     * @param value - значение параметра конфигурации
     */
    void setAttr(const std::string &name, const std::string &attrName, const std::string &value);

    /**
     * Устанавливает значение атрибута параметра конфигурации
     * @param path - путь к параметру конфигурации
     * @param attrName - имя атрибута
     * @param value - значение параметра конфигурации
     */
    void setAttr(const Path &path, const std::string &attrName, const std::string &value);

    /**
     * Устанавливает значение параметра конфигурации
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий узлу параметра конфигурации в дереве
     * параметров конфигурации
     */
    template<typename T> Configuration set(const std::string &name, const T &value)
    {
        return set(name, boost::lexical_cast<std::string>(value));
    }

    /**
     * Устанавливает значение параметра конфигурации
     * @param path - путь к параметру конфигурации
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий узлу параметра конфигурации в дереве
     * параметров конфигурации
     */
    template<typename T> Configuration set(const Path &path, const T &value)
    {
        return set(path, boost::lexical_cast<std::string>(value));
    }

    /**
     * Устанавливает значение параметра конфигурации
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий узлу параметра конфигурации в дереве
     * параметров конфигурации
     */
    Configuration set(const std::string &name, const std::string &value);

    /**
     * Устанавливает значение параметра конфигурации
     * @param path - путь к параметру конфигурации
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий узлу параметра конфигурации в дереве
     * параметров конфигурации
     */
    Configuration set(const Path &path, const std::string &value);

    /**
     * Добавляет новый узел в дерево конфигурации
     * @param name - имя узла в виде "x.y.z"
     * @return объект @ref Configuration, соответствующий созданному узлу в дереве
     * параметров конфигурации
     */
    Configuration add(const std::string &name);

    /**
     * Добавляет новый узел в дерево конфигурации
     * @param path - путь к новому узлу в дереве параметров конфигурации
     * @return объект @ref Configuration, соответствующий созданному узлу в дереве
     * параметров конфигурации
     */
    Configuration add(const Path &path);

    /**
     * Добавляет новый узел в дерево конфигурации и задает соответствующее ему значение
     * @param name - имя узла в виде "x.y.z"
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий созданному узлу в дереве
     * параметров конфигурации
     */
    Configuration add(const std::string &name, const std::string &value);

    /**
     * Добавляет новый узел в дерево конфигурации и задает соответствующее ему значение
     * @param name - путь к новому узлу в дереве параметров конфигурации
     * @param value - значение параметра конфигурации
     * @return объект @ref Configuration, соответствующий созданному узлу в дереве
     * параметров конфигурации
     */
    Configuration add(const Path &path, const std::string &value);

    /**
     * Удаляет узел со всеми потомками из дерева параметров конфигурации
     * @param name - имя узла в виде "x.y.z"
     */
    void remove(const std::string &name);

    /**
     * Удаляет узел со всеми потомками из дерева параметров конфигурации
     * @param path - путь к узлу в дереве параметров конфигурации
     */
    void remove(const Path &path);

    /**
     * Создает объект @ref Configuration, соответствующий узлу дерева параметров конфигурации,
     * путь к которму указан в path
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return @ref Configuration
     */
    Configuration branch(const std::string &path);

    /**
     * Создает объект @ref Configuration, соответствующий узлу дерева параметров конфигурации,
     * путь к которму указан в path
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return @ref Configuration
     */
    Configuration branch(const Path &path);

    /**
     * Возвращет список узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return список узлов
     */
    std::list<Configuration> getListOf(const std::string &name);

    /**
     * Возвращет список узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return список узлов
     */
    std::list<Configuration> getListOf(const Path &path);

    /**
     * Сохраняет в list список узлов дерева параметров конфигурации, соответствующих заданному пути.
     * @param[in] name - путь к узлу в дереве параметров конфигурации
     * @param[out] list - список узлов
     */
    void getListOf(const std::string &name, std::list<Configuration> &list);

    /**
     * Сохраняет в list список узлов дерева параметров конфигурации, соответствующих заданному пути.
     * @param[in] name - путь к узлу в дереве параметров конфигурации
     * @param[out] list - список узлов
     */
    void getListOf(const Path &path, std::list<Configuration> &list);

    /**
     * Возвращет диапазон узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return диапазон узлов
     */
    ConfigurationRange getRangeOf(const std::string &name);

    /**
     * Возвращет диапазон узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return диапазон узлов
     */
    ConfigurationRange getRangeOf(const Path &path);

    /**
     * Возвращает список дочерних узлов
     * @return диапазон дочерних узлов
     */
    ConfigurationRange getChildren();

    /**
     * Осуществляет, если возможно, слияние конфигураций, при этом новые узлы добавляются,
     * существующие перезаписываются. Если слияние невозможно, выбрасывается исключение MergeError.
     * @param other - конфигурация
     */
    void merge(const ConfigurationView &other);

    /**
     * Удаляет все дочерние элементы и значение.
     */
    void clear();

    using ConfigurationView::getRangeOf;
    using ConfigurationView::getChildren;
};

} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_CONFIGURATION_H_ */
