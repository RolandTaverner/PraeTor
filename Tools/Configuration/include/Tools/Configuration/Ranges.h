#ifndef INCLUDE_TOOLS_CONFIGURATION_RANGES_H_
#define INCLUDE_TOOLS_CONFIGURATION_RANGES_H_

// C++
#include <functional>

// Boost
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/shared_ptr.hpp>

#include "Tools/Configuration/RealPath.h"
#include "Tools/Configuration/Store.h"

namespace Tools
{
namespace Configuration
{

class Configuration;
class ConfigurationView;

namespace Detail
{
    class Store;
}

class PathIteratorToConfiguration :
        public std::unary_function<const Detail::RealPath &, Configuration>
{
public:
    explicit PathIteratorToConfiguration(boost::shared_ptr<Detail::Store> storePtr);
    PathIteratorToConfiguration(const PathIteratorToConfiguration &rhs);
    PathIteratorToConfiguration &operator=(const PathIteratorToConfiguration &rhs);

    Configuration operator()(const Detail::RealPath &path) const;

private:
    boost::shared_ptr<Detail::Store> m_storePtr;
};

class PathIteratorToConfigurationView :
        public std::unary_function<const Detail::RealPath &, ConfigurationView>
{
public:
    explicit PathIteratorToConfigurationView(boost::shared_ptr<Detail::Store> storePtr);
    PathIteratorToConfigurationView(const PathIteratorToConfigurationView &rhs);
    PathIteratorToConfigurationView &operator=(const PathIteratorToConfigurationView &rhs);

    ConfigurationView operator()(const Detail::RealPath &path) const;

private:
    boost::shared_ptr<Detail::Store> m_storePtr;
};

typedef boost::transform_iterator<
        PathIteratorToConfiguration,
        Detail::PathIterator,
        Configuration,
        Configuration> ConfigurationIterator;

typedef boost::transform_iterator<
        PathIteratorToConfigurationView,
        Detail::PathIterator,
        ConfigurationView,
        ConfigurationView> ConfigurationViewIterator;

typedef boost::iterator_range<ConfigurationIterator> ConfigurationRange;
typedef boost::iterator_range<ConfigurationViewIterator> ConfigurationViewRange;

} /* namespace Configuration */
} /* namespace Tools */

#endif /* INCLUDE_TOOLS_CONFIGURATION_RANGES_H_ */
