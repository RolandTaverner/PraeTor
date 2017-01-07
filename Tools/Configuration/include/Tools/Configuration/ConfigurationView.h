#ifndef INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONVIEW_H_
#define INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONVIEW_H_

// C++
#include <list>

// Boost
#include <boost/lexical_cast.hpp>

#include "Tools/Configuration/ConfigurationBase.h"
#include "Tools/Configuration/Ranges.h"

namespace Tools
{
namespace Configuration
{
typedef std::pair<std::string, std::string> Attribute;
typedef std::list<Attribute> Attributes;

class ConfigurationView : virtual private ConfigurationBase
{
public:
    /**
     * Конструктор по умолчанию
     */
    ConfigurationView();

    /**
     * Конструктор копирования
     */
    ConfigurationView(const ConfigurationView &rhs);

    /**
     * Конструктор
     * @param confBase - хранилище и базовый путь
     */
    explicit ConfigurationView(const ConfigurationBase &confBase);

    /**
     * Деструктор.
     * Так как @ref Configuration является представлением (view), хранилище данных
     * уничтожается только при уничтожении последнего объекта @ref Configuration,
     * использующего хранилище.
     */
    virtual ~ConfigurationView();

    /**
     * Оператор присваивания
     * @param rhs - другой объект типа @ref Configuration
     * @return ссылку на себя
     */
    ConfigurationView &operator=(const ConfigurationView &rhs);

    /**
     * Возвращает значение, соответствующее имени параметра конфигурации,
     * либо выбрасывает исключение, если не удается однозначно разрешить имя
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @return значение параметра конфигурации
     */
    std::string get(const std::string &name) const;

    /**
     * Возвращает значение, соответствующее пути к параметру конфигурации,
     * либо выбрасывает исключение, если не удается однозначно разрешить путь
     * @param path - путь к параметру конфигурации
     * @return значение параметра конфигурации
     */
    std::string get(const Path &path) const;

    /**
     * Возвращает значение, соответствующее имени параметра конфигурации,
     * либо выбрасывает исключение, если не удается однозначно разрешить имя
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @return значение параметра конфигурации
     */
    template<typename T> T get(const std::string &name) const
    {
        return boost::lexical_cast<T>(get(name));
    }

    /**
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param defaultValue - значение по умолчанию
     * @return значение параметра конфигурации либо заданное значение по умолчанию defaultValue
     */
    template<typename T> T get(const std::string &name, const T &defaultValue) const
    {
        if (exists(name))
        {
            return hasValue(name) ? boost::lexical_cast<T>(get(name)) : defaultValue;
        }
        return defaultValue;
    }

    /**
     * Возвращает значение атрибута, соответствующее имени параметра конфигурации и имени атрибута,
     * либо выбрасывает исключение, если не удается однозначно разрешить имя
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param attrName - имя атрибута
     * @return значение параметра конфигурации
     */
    std::string getAttr(const std::string &name, const std::string &attrName) const;

    /**
     * Возвращает значение атрибута, соответствующее имени параметра конфигурации и имени атрибута,
     * либо выбрасывает исключение, если не удается однозначно разрешить путь
     * @param path - путь к параметру конфигурации
     * @param attrName - имя атрибута
     * @return значение параметра конфигурации
     */
    std::string getAttr(const Path &path, const std::string &attrName) const;

    /**
     * Возвращает значение атрибута, соответствующее имени параметра конфигурации и имени атрибута,
     * либо выбрасывает исключение, если не удается однозначно разрешить имя
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param attrName - имя атрибута
     * @return значение параметра конфигурации
     */
    template<typename T> T getAttr(const std::string &name, const std::string &attrName) const
    {
        return boost::lexical_cast<T>(getAttr(name, attrName));
    }

    /**
     * @param name - имя параметра конфигурации в виде "x.y.z"
     * @param attrName - имя атрибута
     * @param defaultValue - значение по умолчанию
     * @return значение параметра конфигурации либо заданное значение по умолчанию defaultValue
     */
    template<typename T> T getAttr(const std::string &name, const std::string &attrName, const T &defaultValue) const
    {
        if (exists(name) && hasAttr(name, attrName))
        {
            return boost::lexical_cast<T>(getAttr(name, attrName));
        }
        return defaultValue;
    }

    /**
     * Возвращет список атрибутов параметра конфигурации, соответствующих заданному пути.
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return список атрибутов
     */
    Attributes getAttrs(const std::string &name) const;

    /**
     * Возвращет список атрибутов параметра конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return список атрибутов
     */
    Attributes getAttrs(const Path &path) const;

    /**
     * Сохраняет в list список атрибутов параметра конфигурации, соответствующих заданному пути.
     * @param[in] name - путь к узлу в дереве параметров конфигурации
     * @param[out] list - список атрибутов
     */
    void getAttrs(const std::string &name, Attributes &list) const;

    /**
     * Сохраняет в list список атрибутов параметра конфигурации, соответствующих заданному пути.
     * @param[in] name - путь к узлу в дереве параметров конфигурации
     * @param[out] list - список атрибутов
     */
    void getAttrs(const Path &path, Attributes &list) const;

    /**
     * Проверяет, существует ли заданный узел в дереве параметров конфигурации
     * @param name - имя узла в виде "x.y.z"
     * @return true если существует, иначе false
     */
    bool exists(const std::string &name) const;

    /**
     * Проверяет, существует ли заданный узел в дереве параметров конфигурации
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return true если существует, иначе false
     */
    bool exists(const Path &path) const;

    /**
     * Проверяет, задано ли значение для узла в дереве параметров конфигурации
     * @param name - имя узла в виде "x.y.z"
     * @return true если задано, иначе false
     */
    bool hasValue(const std::string &name) const;

    /**
     * Проверяет, задано ли значение для узла в дереве параметров конфигурации
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return true если задано, иначе false
     */
    bool hasValue(const Path &path) const;

    /**
     * Проверяет, задано ли значение атрибута для узла в дереве параметров конфигурации
     * @param name - имя узла в виде "x.y.z"
     * @return true если задано, иначе false
     */
    bool hasAttr(const std::string &name, const std::string &attrName) const;

    /**
     * Проверяет, задано ли значение атрибута для узла в дереве параметров конфигурации
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return true если задано, иначе false
     */
    bool hasAttr(const Path &path, const std::string &attrName) const;

    /**
     * Создает объект @ref Configuration, соответствующий узлу дерева параметров конфигурации,
     * путь к которму указан в path
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return @ref Configuration
     */
    const ConfigurationView branch(const std::string &path) const;

    /**
     * Создает объект @ref Configuration, соответствующий узлу дерева параметров конфигурации,
     * путь к которму указан в path
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return @ref Configuration
     */
    const ConfigurationView branch(const Path &path) const;

    /**
     * Возвращет диапазон узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return диапазон узлов
     */
    ConfigurationViewRange getRangeOf(const std::string &name) const;

    /**
     * Возвращет диапазон узлов дерева параметров конфигурации, соответствующих заданному пути.
     * Например, @code getListOf("config.servers.server") @endcode вернет список узлов
     * "config.servers.server".
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return диапазон узлов
     */
    ConfigurationViewRange getRangeOf(const Path &path) const;

    /**
     * Возвращает список дочерних узлов
     * @return диапазон дочерних узлов
     */
    ConfigurationViewRange getChildren() const;

    /**
     * Возвращает количество элементов, соответствующих указанному пути
     * @param name - путь к узлу в дереве параметров конфигурации
     * @return количество элементов
     */
    std::size_t getCountOf(const std::string &name) const;

    /**
     * Возвращает количество элементов, соответствующих указанному пути
     * @param path - путь к узлу в дереве параметров конфигурации
     * @return количество элементов
     */
    std::size_t getCountOf(const Path &path) const;

    /**
     * Возвращает путь в дереве параметров конфигурации
     * @return путь в дереве параметров конфигурации
     */
    std::string location() const;

    /**
     * Возвращает путь в дереве параметров конфигурации
     * @return путь в дереве параметров конфигурации
     */
    Path pathLocation() const;
};

} /* namespace Configuration */
} /* namespace Tools */

#endif /* INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONVIEW_H_ */
