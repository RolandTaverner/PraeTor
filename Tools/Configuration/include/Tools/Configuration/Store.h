#ifndef TOOLS_CONFIGURATION_STORE_H_
#define TOOLS_CONFIGURATION_STORE_H_

// C++
#include <functional>
#include <list>

// Boost
#include <boost/assert.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_ptr.hpp>

#include "Tools/Configuration/Errors.h"
#include "Tools/Configuration/Path.h"
#include "Tools/Configuration/RealPath.h"

namespace Tools
{
namespace Configuration
{
namespace Detail
{

class IsConfNode :
        public std::unary_function<boost::property_tree::ptree::const_assoc_iterator::reference, bool>
{
public:
    bool operator()(argument_type val) const
    {
        return val.second.count("<xmlattr>") == 1u;
    }
};

typedef boost::filter_iterator<IsConfNode, boost::property_tree::ptree::const_assoc_iterator> ConfNodeFilterIterator;

class PathIterator
  : public boost::iterator_adaptor<
            PathIterator,                                         // Derived iterator type
            ConfNodeFilterIterator,                               // Base iterator type
            const RealPath,                                       // Value type
            boost::forward_traversal_tag,                         // CategoryOrTraversal
            const RealPath>                                       // Reference type
{
public:
    PathIterator() :
        PathIterator::iterator_adaptor_()
    {
    }

    PathIterator(const RealPath &commonPath,
                 const ConfNodeFilterIterator &p) :
         PathIterator::iterator_adaptor_(p), m_commonPath(commonPath)
    {
    }

    PathIterator(const PathIterator &other) :
        PathIterator::iterator_adaptor_(other.base()),
        m_commonPath(other.m_commonPath)
    {
    }

private:
    friend class boost::iterator_core_access;

    iterator_adaptor_::reference dereference() const
    {
        const boost::property_tree::ptree::const_assoc_iterator &j = base().base();

        BOOST_ASSERT(j->second.count("<xmlattr>") == 1u);
        boost::property_tree::ptree::const_assoc_iterator itAttr = j->second.find("<xmlattr>");

        BOOST_ASSERT(itAttr->second.count("ConfigurationInternalID") == 1u);
        boost::property_tree::ptree::const_assoc_iterator itId = itAttr->second.find("ConfigurationInternalID");

        return m_commonPath / RealPath::PathElement(j->first, itId->second.data());
    }

    RealPath m_commonPath;
};

typedef boost::iterator_range<PathIterator> PathIteratorRange;

class Store
{
public:
    Store();
    virtual ~Store();

    RealPath getRealPath(const RealPath &path, const Path &tail) const;
    RealPath getRealPath(const Path &path) const;
    std::string get(const RealPath &path) const;
    void getListOf(const RealPath &path, const std::string &listName, std::list<RealPath> &paths) const;
    PathIteratorRange getRangeOf(const RealPath &path, const std::string &listName) const;
    std::size_t getCountOf(const RealPath &path, const std::string &listName) const;

    void getChildNodes(const RealPath &path, std::list<RealPath> &paths) const;
    PathIteratorRange getChildRange(const RealPath &path) const;

    RealPath set(const RealPath &path, const Path &tail, const std::string &value);
    RealPath add(const RealPath &path, const Path &tail);
    void remove(const RealPath &path);

    std::string getAttr(const RealPath &path, const std::string &attrName) const;
    bool hasAttr(const RealPath &path, const std::string &attrName) const;
    void setAttr(const RealPath &path, const std::string &attrName, const std::string &attrValue);
    void getAttrList(const RealPath &path, std::list<std::pair<std::string, std::string> > &attrs) const;

private:
    const boost::property_tree::ptree *getNodeOf(const RealPath &path,
                                                 const boost::property_tree::ptree *currentNode) const;
    boost::property_tree::ptree *getNodeOf(const RealPath &path,
                                           boost::property_tree::ptree *currentNode);

private:
    boost::property_tree::ptree m_configTree;
    std::size_t m_id;
};

typedef boost::shared_ptr<Store> StorePtr;

} /* namespace Detail */
} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_STORE_H_ */
