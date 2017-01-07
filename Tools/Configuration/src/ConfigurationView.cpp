#include "Tools/Configuration/ConfigurationView.h"
#include "Tools/Configuration/Store.h"

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
ConfigurationView::ConfigurationView()
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationView::ConfigurationView(const ConfigurationView &rhs):
        ConfigurationBase(rhs)
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationView::ConfigurationView(const ConfigurationBase &confBase):
        ConfigurationBase(confBase)
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationView::~ConfigurationView()
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationView &ConfigurationView::operator=(const ConfigurationView &rhs)
{
    ConfigurationBase::operator=(rhs);
    return *this;
}

//--------------------------------------------------------------------------------------------------
std::string ConfigurationView::get(const std::string &name) const
{
    return get(Path(name));
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::exists(const std::string &name) const
{
    return exists(Path(name));
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::hasValue(const std::string &name) const
{
    return hasValue(Path(name));
}

//--------------------------------------------------------------------------------------------------
const ConfigurationView ConfigurationView::branch(const std::string &path) const
{
    return branch(Path(path));
}

//--------------------------------------------------------------------------------------------------
std::string ConfigurationView::get(const Path &path) const
{
    const Detail::RealPath valuePath = getStore().getRealPath(getPath(), path);
    return getStore().get(valuePath);
}

//--------------------------------------------------------------------------------------------------
const ConfigurationView ConfigurationView::branch(const Path &path) const
{
    const Detail::RealPath branchPath = getStore().getRealPath(getPath(), path);
    return ConfigurationView(ConfigurationBase(m_storePtr, branchPath));
}

//--------------------------------------------------------------------------------------------------
ConfigurationViewRange ConfigurationView::getRangeOf(const std::string &name) const
{
    return getRangeOf(Path(name));
}

//--------------------------------------------------------------------------------------------------
ConfigurationViewRange ConfigurationView::getRangeOf(const Path &path) const
{
    if (path.isRoot())
    {
        throw std::invalid_argument("Path can't be empty");
    }

    const Detail::RealPath pathToList = getStore().getRealPath(getPath(), path.parent());
    const std::string listName = path.getElements().back();

    Detail::PathIteratorRange pathRange = getStore().getRangeOf(pathToList, listName);

    return ConfigurationViewRange(ConfigurationViewIterator(pathRange.begin(), PathIteratorToConfigurationView(m_storePtr)),
                                  ConfigurationViewIterator(pathRange.end(), PathIteratorToConfigurationView(m_storePtr)));
}

//--------------------------------------------------------------------------------------------------
std::size_t ConfigurationView::getCountOf(const std::string &name) const
{
    return getCountOf(Path(name));
}

//--------------------------------------------------------------------------------------------------
std::size_t ConfigurationView::getCountOf(const Path &path) const
{
    if (path.isRoot())
    {
        throw std::invalid_argument("Path can't be empty");
    }
    const std::string listName = path.getElements().back();
    const Detail::RealPath pathToList = getStore().getRealPath(getPath(), path.parent());

    return getStore().getCountOf(pathToList, listName);
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::exists(const Path &path) const
{
    try
    {
        getStore().getRealPath(getPath(), path);
    }
    catch(const PathNotExistsError &)
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::hasValue(const Path &path) const
{
    return !get(path).empty();
}

//--------------------------------------------------------------------------------------------------
std::string ConfigurationView::location() const
{
    return getPath().toPath().string();
}

//--------------------------------------------------------------------------------------------------
Path ConfigurationView::pathLocation() const
{
    return getPath().toPath();
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::hasAttr(const std::string &name, const std::string &attrName) const
{
    return hasAttr(Path(name), attrName);
}

//--------------------------------------------------------------------------------------------------
bool ConfigurationView::hasAttr(const Path &path, const std::string &attrName) const
{
    const Detail::RealPath valuePath = getStore().getRealPath(getPath(), path);
    return getStore().hasAttr(valuePath, attrName);
}

//--------------------------------------------------------------------------------------------------
std::string ConfigurationView::getAttr(const std::string &name, const std::string &attrName) const
{
    return getAttr(Path(name), attrName);
}

//--------------------------------------------------------------------------------------------------
std::string ConfigurationView::getAttr(const Path &path, const std::string &attrName) const
{
    const Detail::RealPath valuePath = getStore().getRealPath(getPath(), path);
    return getStore().getAttr(valuePath, attrName);
}

//--------------------------------------------------------------------------------------------------
Attributes ConfigurationView::getAttrs(const std::string &name) const
{
    return getAttrs(Path(name));
}

//--------------------------------------------------------------------------------------------------
Attributes ConfigurationView::getAttrs(const Path &path) const
{
    Attributes list;
    getAttrs(path, list);
    return list;
}

//--------------------------------------------------------------------------------------------------
void ConfigurationView::getAttrs(const std::string &name, Attributes &list) const
{
    getAttrs(Path(name), list);
}

//--------------------------------------------------------------------------------------------------
void ConfigurationView::getAttrs(const Path &path, Attributes &list) const
{
    const Detail::RealPath valuePath = getStore().getRealPath(getPath(), path);
    getStore().getAttrList(valuePath, list);
}

//--------------------------------------------------------------------------------------------------
ConfigurationViewRange ConfigurationView::getChildren() const
{
    Detail::PathIteratorRange pathRange = getStore().getChildRange(getPath());

    return ConfigurationViewRange(ConfigurationViewIterator(pathRange.begin(), PathIteratorToConfigurationView(m_storePtr)),
                                  ConfigurationViewIterator(pathRange.end(), PathIteratorToConfigurationView(m_storePtr)));
}

} /* namespace Configuration */
} /* namespace Tools */
