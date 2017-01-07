// Boost
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include "Tools/Configuration/Store.h"

namespace pt = boost::property_tree;

namespace Tools
{
namespace Configuration
{
namespace Detail
{

//--------------------------------------------------------------------------------------------------
Store::Store():
        m_id(0u)
{
}

//--------------------------------------------------------------------------------------------------
Store::~Store()
{
}

//--------------------------------------------------------------------------------------------------
const pt::ptree* Store::getNodeOf(const RealPath &path, const pt::ptree *startNode) const
{
    const RealPath::PathElements &realPathElements = path.getElements();
    for (RealPath::PathElements::const_iterator itRealPath = realPathElements.begin();
            itRealPath != realPathElements.end(); ++itRealPath)
    {
        const pt::ptree *nextNode = NULL;
        std::pair<pt::ptree::const_assoc_iterator, pt::ptree::const_assoc_iterator> range =
                startNode->equal_range(itRealPath->first);
        pt::ptree::const_assoc_iterator j = range.first;
        for (; j != range.second; ++j)
        {
            BOOST_ASSERT(j->second.count("<xmlattr>") == 1u);
            pt::ptree::const_assoc_iterator itAttr = j->second.find("<xmlattr>");

            BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
            pt::ptree::const_assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

            if (itId->second.data() == itRealPath->second)
            {
                nextNode = &j->second;
                break;
            }
        }

        if (nextNode == NULL)
        {
            throw StoreChangedError("Can't resolve path because store changed: " + path.string(),
                                    path.string());
        }

        startNode = nextNode;
    }
    return startNode;
}

//--------------------------------------------------------------------------------------------------
pt::ptree* Store::getNodeOf(const RealPath &path, pt::ptree *startNode)
{
    pt::ptree *curentNode = startNode;

    const RealPath::PathElements& realPathElements = path.getElements();
    for (RealPath::PathElements::const_iterator itRealPath = realPathElements.begin();
            itRealPath != realPathElements.end(); ++itRealPath)
    {
        pt::ptree *nextNode = NULL;
        std::pair<pt::ptree::assoc_iterator, pt::ptree::assoc_iterator> range =
                curentNode->equal_range(itRealPath->first);
        pt::ptree::assoc_iterator j = range.first;
        for (; j != range.second; ++j)
        {
            BOOST_ASSERT(j->second.count("<xmlattr>") == 1u);
            pt::ptree::assoc_iterator itAttr = j->second.find("<xmlattr>");

            BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
            pt::ptree::assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

            if (itId->second.data() == itRealPath->second)
            {
                nextNode = &j->second;
                break;
            }
        }

        if (nextNode == NULL)
        {
            throw StoreChangedError("Can't resolve path because store changed: " + path.string(),
                                    path.string());
        }

        curentNode = nextNode;
    }
    return curentNode;
}

//--------------------------------------------------------------------------------------------------
RealPath Store::getRealPath(const RealPath &path, const Path &tail) const
{
    const pt::ptree *currentNode = getNodeOf(path, &m_configTree);
    RealPath fullPath(path);

    const Path::PathElements &tailElements = tail.getElements();
    for (Path::PathElements::const_iterator itPath = tailElements.begin();
            itPath != tailElements.end();
            ++itPath)
    {
        if (currentNode->count(*itPath) == 0)
        {
            throw PathNotExistsError("Can't resolve Path: " + path.string() + "." + tail.string(),
                                     path.string() + "." + tail.string());
        }
        else if (currentNode->count(*itPath) > 1)
        {
            throw AmbiguousPathError("Can't resolve Path because of multiple choices: " + path.string() + "." + tail.string(),
                                     path.string() + "." + tail.string());
        }

        pt::ptree::const_assoc_iterator itNode = currentNode->find(*itPath);

        BOOST_ASSERT(itNode->second.count("<xmlattr>") == 1u);
        pt::ptree::const_assoc_iterator itAttr = itNode->second.find("<xmlattr>");

        BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
        pt::ptree::const_assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

        fullPath = fullPath / RealPath::PathElement(*itPath, itId->second.data());
        currentNode = &itNode->second;
    }

    return fullPath;
}

//--------------------------------------------------------------------------------------------------
RealPath Store::getRealPath(const Path &path) const
{
    RealPath fullPath;

    const pt::ptree *currentNode = &m_configTree;

    const Path::PathElements &pathElements = path.getElements();
    for (Path::PathElements::const_iterator itPath = pathElements.begin();
            itPath != pathElements.end();
            ++itPath)
    {
        if (currentNode->count(*itPath) == 0)
        {
            throw PathNotExistsError("Can't resolve Path: " + path.string(),
                                     path.string());
        }
        else if (currentNode->count(*itPath) > 1)
        {
            throw AmbiguousPathError("Can't resolve Path because of multiple choices: " + path.string(),
                                     path.string());
        }

        pt::ptree::const_assoc_iterator itNode = currentNode->find(*itPath);

        BOOST_ASSERT(itNode->second.count("<xmlattr>") == 1u);
        pt::ptree::const_assoc_iterator itAttr = itNode->second.find("<xmlattr>");

        BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
        pt::ptree::const_assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

        fullPath = fullPath / RealPath::PathElement(*itPath, itId->second.data());
        currentNode = &itNode->second;
    }

    return fullPath;
}

//--------------------------------------------------------------------------------------------------
void Store::getListOf(const RealPath &path, const std::string &listName, std::list<RealPath> &paths) const
{
    const pt::ptree *currentNode = getNodeOf(path, &m_configTree);

    std::pair<pt::ptree::const_assoc_iterator, pt::ptree::const_assoc_iterator> range =
            currentNode->equal_range(listName);
    pt::ptree::const_assoc_iterator j = range.first;
    for (; j != range.second; ++j)
    {
        BOOST_ASSERT(j->second.count("<xmlattr>") == 1u);
        pt::ptree::const_assoc_iterator itAttr = j->second.find("<xmlattr>");

        BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
        pt::ptree::const_assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

        const RealPath itemPath = path / RealPath::PathElement(listName, itId->second.data());
        paths.push_back(itemPath);
    }
}

typedef std::pair<pt::ptree::const_assoc_iterator, pt::ptree::const_assoc_iterator> PTreeRange;

PathIteratorRange ptreeRangeToPathIteratorRange(const RealPath &path,
                                                const PTreeRange &ptreeRange,
                                                const pt::ptree::const_assoc_iterator &end)
{
    ConfNodeFilterIterator confNodeBegin(IsConfNode(), ptreeRange.first, end);
    ConfNodeFilterIterator confNodeEnd(IsConfNode(), ptreeRange.second, end);
    return PathIteratorRange(PathIterator(path, confNodeBegin), PathIterator(path, confNodeEnd));
}

//--------------------------------------------------------------------------------------------------
PathIteratorRange Store::getRangeOf(const RealPath &path, const std::string &listName) const
{
    const pt::ptree *currentNode = getNodeOf(path, &m_configTree);

    return ptreeRangeToPathIteratorRange(path,
                                         currentNode->equal_range(listName),
                                         currentNode->not_found());
}

//--------------------------------------------------------------------------------------------------
class RealPathCountStub
{
public:
    bool operator()(const RealPath &) const
    {
        return true;
    }
};

std::size_t Store::getCountOf(const RealPath &path, const std::string &listName) const
{
    return boost::range::count_if(getRangeOf(path, listName), RealPathCountStub());
}

//--------------------------------------------------------------------------------------------------
std::string Store::get(const RealPath &path) const
{
    return getNodeOf(path, &m_configTree)->data();
}

//--------------------------------------------------------------------------------------------------
RealPath Store::set(const RealPath &path, const Path &tail, const std::string &value)
{
    pt::ptree *currentNode = getNodeOf(path, &m_configTree);
    RealPath fullPath(path);

    const Path::PathElements &pathElements = tail.getElements();
    for (Path::PathElements::const_iterator itPath = pathElements.begin();
            itPath != pathElements.end();
            ++itPath)
    {
        if (currentNode->count(*itPath) > 1)
        {
            throw AmbiguousPathError("Can't resolve Path because of multiple choices: " + path.string(),
                                     path.string());
        }
        else if (currentNode->count(*itPath) == 0)
        {
            currentNode->add_child(*itPath, pt::ptree()).put("<xmlattr>.ConfigurationInternalID", m_id++);
        }

        pt::ptree::assoc_iterator itNode = currentNode->find(*itPath);
        currentNode = &itNode->second;
        fullPath = fullPath / RealPath::PathElement(*itPath, currentNode->get<std::string>("<xmlattr>.ConfigurationInternalID"));
    }

    currentNode->put_value(value);
    return fullPath;
}

//--------------------------------------------------------------------------------------------------
RealPath Store::add(const RealPath &path, const Path &tail)
{
    pt::ptree *currentNode = getNodeOf(path, &m_configTree);
    RealPath fullPath(path);

    const Path::PathElements &pathElements = tail.getElements();
    for (Path::PathElements::const_iterator itPath = pathElements.begin();
            itPath != pathElements.end();
            ++itPath)
    {
        currentNode = &currentNode->add_child(*itPath, pt::ptree());
        currentNode->put("<xmlattr>.ConfigurationInternalID", m_id++);
        fullPath = fullPath / RealPath::PathElement(*itPath, currentNode->get<std::string>("<xmlattr>.ConfigurationInternalID"));
    }
    return fullPath;
}

//--------------------------------------------------------------------------------------------------
void Store::getChildNodes(const RealPath &path, std::list<RealPath> &paths) const
{
    const pt::ptree *currentNode = getNodeOf(path, &m_configTree);
    pt::ptree::const_iterator i = currentNode->begin();
    for(; i != currentNode->end(); ++i)
    {
        if (i->first == "<xmlattr>")
        {
            continue;
        }
        RealPath node(path / RealPath::PathElement(i->first, i->second.get<std::string>("<xmlattr>.ConfigurationInternalID")));
        paths.push_back(node);
    }
}

//--------------------------------------------------------------------------------------------------
PathIteratorRange Store::getChildRange(const RealPath &path) const
{
    const pt::ptree *currentNode = getNodeOf(path, &m_configTree);

    std::pair<pt::ptree::const_assoc_iterator, pt::ptree::const_assoc_iterator> range(
            currentNode->ordered_begin(), currentNode->not_found());

    return ptreeRangeToPathIteratorRange(path, range, currentNode->not_found());
}

//--------------------------------------------------------------------------------------------------
void Store::remove(const RealPath &path)
{
    if (path.isRoot())
    {
        throw RemoveError("Can't remove root");
    }
    pt::ptree *parentNode = getNodeOf(path.parent(), &m_configTree);

    std::pair<pt::ptree::assoc_iterator, pt::ptree::assoc_iterator> range =
            parentNode->equal_range(path.getElements().back().first);
    pt::ptree::assoc_iterator j = range.first;
    for (; j != range.second; ++j)
    {
        BOOST_ASSERT(j->second.count("<xmlattr>") == 1u);
        pt::ptree::assoc_iterator itAttr = j->second.find("<xmlattr>");

        BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
        pt::ptree::assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

        if (itId->second.data() == path.getElements().back().second)
        {
            pt::ptree::iterator itRemove = parentNode->to_iterator(j);
            parentNode->erase(itRemove);
            return;
        }
    }
    BOOST_ASSERT_MSG(false, "Node to remove not found");
}

//--------------------------------------------------------------------------------------------------
std::string Store::getAttr(const RealPath &path, const std::string &attrName) const
{
    const pt::ptree *node = getNodeOf(path, &m_configTree);

    BOOST_ASSERT(node->count("<xmlattr>") == 1u);
    pt::ptree::const_assoc_iterator itAttrs = node->find("<xmlattr>");

    BOOST_ASSERT(itAttrs->second.count(attrName) <= 1u);
    pt::ptree::const_assoc_iterator itAttrValue = itAttrs->second.find(attrName);

    if (itAttrValue == itAttrs->second.not_found())
    {
        throw AttrNotFoundError("Attribute " + attrName + " not found at " + path.string(), path.string(), attrName);
    }

    return itAttrValue->second.data();
}

//--------------------------------------------------------------------------------------------------
bool Store::hasAttr(const RealPath &path, const std::string &attrName) const
{
    const pt::ptree *node = getNodeOf(path, &m_configTree);

    BOOST_ASSERT(node->count("<xmlattr>") == 1u);
    pt::ptree::const_assoc_iterator itAttrs = node->find("<xmlattr>");

    BOOST_ASSERT(itAttrs->second.count(attrName) <= 1u);
    pt::ptree::const_assoc_iterator itAttrValue = itAttrs->second.find(attrName);

    return itAttrValue != itAttrs->second.not_found();
}

//--------------------------------------------------------------------------------------------------
void Store::setAttr(const RealPath &path, const std::string &attrName, const std::string &attrValue)
{
    pt::ptree *node = getNodeOf(path, &m_configTree);

    BOOST_ASSERT(node->count("<xmlattr>") == 1u);
    pt::ptree::assoc_iterator itAttrs = node->find("<xmlattr>");
    pt::ptree::assoc_iterator itAttrValue = itAttrs->second.find(attrName);

    if (itAttrValue != itAttrs->second.not_found())
    {
        itAttrValue->second.put_value(attrValue);
        return;
    }

    itAttrs->second.add_child(attrName, pt::ptree()).put_value(attrValue);
}

//--------------------------------------------------------------------------------------------------
void Store::getAttrList(const RealPath &path, std::list<std::pair<std::string, std::string> > &attrs) const
{
    if (path.isRoot())
    {
        return;
    }

    const pt::ptree *node = getNodeOf(path, &m_configTree);

    BOOST_ASSERT(node->count("<xmlattr>") == 1u);
    pt::ptree::const_assoc_iterator itAttrs = node->find("<xmlattr>");

    for(pt::ptree::const_assoc_iterator i = itAttrs->second.ordered_begin();
            i != itAttrs->second.not_found();
            ++i)
    {
        if (i->first == "ConfigurationInternalID")
        {
            continue;
        }
        attrs.push_back(std::make_pair(i->first, i->second.data()));
    }
}

} /* namespace Detail */
} /* namespace Configuration */
} /* namespace Tools */
