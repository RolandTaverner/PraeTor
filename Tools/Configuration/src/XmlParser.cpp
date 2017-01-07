// C++
#include <fstream>

// Boost
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Tools/Configuration/XmlParser.h"

namespace pt = boost::property_tree;

namespace Tools
{
namespace Configuration
{
namespace Parsers
{
namespace Xml
{

namespace Detail
{

void processNode(Configuration &conf, const std::string &nodeName, const pt::ptree &node)
{
    if (nodeName != "<xmlattr>")
    {
        Configuration newNode = conf.add(nodeName);
        if (!node.data().empty())
        {
            newNode.set("", node.data());
        }
        pt::ptree::const_iterator i = node.begin();
        for (; i != node.end(); ++i)
        {
            Detail::processNode(newNode, i->first, i->second);
        }
    }
    else
    {
        pt::ptree::const_iterator i = node.begin();
        for (; i != node.end(); ++i)
        {
            conf.setAttr(Path(), i->first, i->second.data());
        }
    }
}

void saveNode(pt::ptree &saveTreeNode, ConfigurationView confNode)
{
    if (!confNode.pathLocation().isRoot())
    {
        const std::string nodeName = confNode.pathLocation().getElements().back();
        pt::ptree &newSaveNode = saveTreeNode.add_child(nodeName, pt::ptree());
        if (confNode.hasValue(Path()))
        {
            newSaveNode.put_value(confNode.get(Path()));
        }

        Attributes attrs;
        confNode.getAttrs(Path(), attrs);
        for(Attributes::const_iterator i = attrs.begin(); i != attrs.end(); ++i)
        {
            newSaveNode.put("<xmlattr>." + i->first, i->second);
        }

        ConfigurationViewRange children = confNode.getChildren();
        BOOST_FOREACH(ConfigurationView child, children)
        {
            saveNode(newSaveNode, child);
        }
    }
    else
    {
        ConfigurationViewRange children = confNode.getChildren();
        BOOST_FOREACH(ConfigurationView child, children)
        {
            saveNode(saveTreeNode, child);
        }
    }
}

} /* namespace Detail */

//--------------------------------------------------------------------------------------------------
Configuration readXml(const std::string &filePath)
{
    std::ifstream file(filePath.c_str());
    if (!file)
    {
        throw std::runtime_error("Can't open file " + filePath);
    }
    return readXml(file);
}

//--------------------------------------------------------------------------------------------------
Configuration readXml(std::istream &stream)
{
    Configuration conf;
    pt::ptree tree;
    pt::xml_parser::read_xml(stream, tree,
                             pt::xml_parser::no_comments | pt::xml_parser::trim_whitespace);
    pt::ptree::const_iterator i = tree.begin();
    for (; i != tree.end(); ++i)
    {
        Detail::processNode(conf, i->first, i->second);
    }
    return conf;
}

//--------------------------------------------------------------------------------------------------
void writeXml(const ConfigurationView &conf, const std::string &filePath)
{
    std::ofstream file(filePath.c_str(), std::ios_base::trunc);
    if (!file)
    {
        throw std::runtime_error("Can't open file " + filePath);
    }
    writeXml(conf, file);
}

//--------------------------------------------------------------------------------------------------
void writeXml(const ConfigurationView &conf, std::ostream &stream)
{
    pt::ptree saveTree;
    Detail::saveNode(saveTree, conf);
#if BOOST_VERSION >= 105600
    pt::write_xml(stream, saveTree, pt::xml_writer_make_settings<pt::ptree::key_type>(' ', 4));
#else
    pt::write_xml(stream, saveTree, pt::xml_writer_make_settings(' ', 4));
#endif
}

} /* namespace Xml */
} /* namespace Parsers */
} /* namespace Configuration */
} /* namespace Tools */
