// C++
#include <map>

// Boost
#include <boost/foreach.hpp>

#include "Tools/Configuration/Configuration.h"
#include "Tools/Configuration/Store.h"

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
Configuration::Configuration()
{
}

//--------------------------------------------------------------------------------------------------
Configuration::Configuration(const Configuration &rhs):
        ConfigurationBase(rhs), ConfigurationView(rhs)
{
}

//--------------------------------------------------------------------------------------------------
Configuration::Configuration(const ConfigurationBase &confBase):
        ConfigurationBase(confBase)
{
}

//--------------------------------------------------------------------------------------------------
Configuration::~Configuration()
{
}

//--------------------------------------------------------------------------------------------------
Configuration &Configuration::operator=(const Configuration &rhs)
{
    ConfigurationBase::operator=(rhs);
    return *this;
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::set(const std::string &name, const std::string &value)
{
    return set(Path(name), value);
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::add(const std::string &name)
{
    return add(Path(name));
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::add(const std::string &name, const std::string &value)
{
    return add(Path(name), value);
}

//--------------------------------------------------------------------------------------------------
void Configuration::remove(const std::string &name)
{
    remove(Path(name));
}

//--------------------------------------------------------------------------------------------------
void Configuration::clear()
{
    ConfigurationRange children = getChildren();
    BOOST_FOREACH(Configuration conf, children)
    {
        conf.remove(Path());
    }

    set(Path(), "");
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::branch(const std::string &path)
{
    return branch(Path(path));
}

//--------------------------------------------------------------------------------------------------
std::list<Configuration> Configuration::getListOf(const std::string &path)
{
    return getListOf(Path(path));
}

//--------------------------------------------------------------------------------------------------
void Configuration::getListOf(const std::string &name, std::list<Configuration> &list)
{
    getListOf(Path(name), list);
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::set(const Path &path, const std::string &value)
{
    return Configuration(ConfigurationBase(m_storePtr, getStore().set(getPath(), path, value)));
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::add(const Path &path)
{
    return Configuration(ConfigurationBase(m_storePtr, getStore().add(getPath(), path)));
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::add(const Path &path, const std::string &value)
{
    return add(path).set(Path(), value);
}

//--------------------------------------------------------------------------------------------------
void Configuration::remove(const Path &path)
{
    const Detail::RealPath pathToRemove = getStore().getRealPath(getPath(), path);
    getStore().remove(pathToRemove);
}

//--------------------------------------------------------------------------------------------------
Configuration Configuration::branch(const Path &path)
{
    const Detail::RealPath branchPath = getStore().getRealPath(getPath(), path);
    return Configuration(ConfigurationBase(m_storePtr, branchPath));
}

//--------------------------------------------------------------------------------------------------
std::list<Configuration> Configuration::getListOf(const Path &path)
{
    std::list<Configuration> list;
    getListOf(path, list);
    return list;
}

//--------------------------------------------------------------------------------------------------
void Configuration::getListOf(const Path &path, std::list<Configuration> &list)
{
    if (path.isRoot())
    {
        throw std::invalid_argument("Path can't be empty");
    }
    const std::string listName = path.getElements().back();
    const Detail::RealPath pathToList = getStore().getRealPath(getPath(), path.parent());
    std::list<Detail::RealPath> paths;
    getStore().getListOf(pathToList, listName, paths);
    for(std::list<Detail::RealPath>::const_iterator i = paths.begin(); i != paths.end(); ++i)
    {
        list.push_back(Configuration(ConfigurationBase(m_storePtr, *i)));
    }
}

//--------------------------------------------------------------------------------------------------
ConfigurationRange Configuration::getRangeOf(const std::string &name)
{
    return getRangeOf(Path(name));
}

//--------------------------------------------------------------------------------------------------
ConfigurationRange Configuration::getRangeOf(const Path &path)
{
    if (path.isRoot())
    {
        throw std::invalid_argument("Path can't be empty");
    }

    const Detail::RealPath pathToList = getStore().getRealPath(getPath(), path.parent());
    const std::string listName = path.getElements().back();

    Detail::PathIteratorRange pathRange = getStore().getRangeOf(pathToList, listName);

    return ConfigurationRange(ConfigurationIterator(pathRange.begin(), PathIteratorToConfiguration(m_storePtr)),
                              ConfigurationIterator(pathRange.end(), PathIteratorToConfiguration(m_storePtr)));
}

//--------------------------------------------------------------------------------------------------
void Configuration::merge(const ConfigurationView &other)
{
    if (other.hasValue(Path()))
    {
        set(Path(), other.get(Path()));
    }

    Attributes attrs;
    other.getAttrs(Path(), attrs);
    for(Attributes::const_iterator itAttrs = attrs.begin(); itAttrs != attrs.end(); ++itAttrs)
    {
        setAttr(Path(), itAttrs->first, itAttrs->second);
    }

    ConfigurationViewRange children = other.getChildren();

    std::map<std::string, std::size_t> nodeCount;
    std::map<std::string, std::size_t> otherNodeCount;

    BOOST_FOREACH(ConfigurationView child, children)
    {
        const std::string node = child.pathLocation().getElements().back();
        otherNodeCount[node] += 1u;
        nodeCount[node] = getCountOf(node);
    }

    BOOST_FOREACH(ConfigurationView node, children)
    {
        Configuration newNode;
        const std::string nodeName = node.pathLocation().getElements().back();
        if (nodeCount[nodeName] > 1u)
        {
            throw MergeError("Can't merge configurations");
        }
        else if (nodeCount[nodeName] == 1u && otherNodeCount[nodeName] > 1u)
        {
            throw MergeError("Can't merge configurations");
        }
        else if (nodeCount[nodeName] == 1u && otherNodeCount[nodeName] == 1u)
        {
            if (node.hasValue(Path()))
            {
                newNode = set(Path(nodeName), node.get(Path()));
            }
            else
            {
                newNode = branch(Path(nodeName));
            }
        }
        else
        {
            newNode = add(Path(nodeName), node.get(Path()));
        }

        newNode.merge(node);
    }
}

//--------------------------------------------------------------------------------------------------
void Configuration::setAttr(const std::string &name, const std::string &attrName, const std::string &value)
{
    setAttr(Path(name), attrName, value);
}

//--------------------------------------------------------------------------------------------------
void Configuration::setAttr(const Path &path, const std::string &attrName, const std::string &value)
{
    const Detail::RealPath valuePath = getStore().getRealPath(getPath(), path);
    getStore().setAttr(valuePath, attrName, value);
}

//--------------------------------------------------------------------------------------------------
ConfigurationRange Configuration::getChildren()
{
    Detail::PathIteratorRange pathRange = getStore().getChildRange(getPath());

    return ConfigurationRange(ConfigurationIterator(pathRange.begin(), PathIteratorToConfiguration(m_storePtr)),
                              ConfigurationIterator(pathRange.end(), PathIteratorToConfiguration(m_storePtr)));
}

} /* namespace Configuration */
} /* namespace Tools */
