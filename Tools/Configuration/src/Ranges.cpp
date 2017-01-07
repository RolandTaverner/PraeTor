// Boost
#include <boost/assert.hpp>

#include "Tools/Configuration/Configuration.h"
#include "Tools/Configuration/ConfigurationView.h"
#include "Tools/Configuration/Ranges.h"
#include "Tools/Configuration/Store.h"

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
PathIteratorToConfiguration::PathIteratorToConfiguration(Detail::StorePtr storePtr):
        m_storePtr(storePtr)
{
    BOOST_ASSERT(m_storePtr.get() != NULL);
}

//--------------------------------------------------------------------------------------------------
PathIteratorToConfiguration::PathIteratorToConfiguration(const PathIteratorToConfiguration &rhs):
        m_storePtr(rhs.m_storePtr)
{
}

//--------------------------------------------------------------------------------------------------
PathIteratorToConfiguration &PathIteratorToConfiguration::operator=(const PathIteratorToConfiguration &rhs)
{
    if (this != &rhs)
    {
        m_storePtr = rhs.m_storePtr;
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
Configuration PathIteratorToConfiguration::operator()(const Detail::RealPath &path) const
{
    return Configuration(ConfigurationBase(m_storePtr, path));
}

//--------------------------------------------------------------------------------------------------
PathIteratorToConfigurationView::PathIteratorToConfigurationView(Detail::StorePtr storePtr):
        m_storePtr(storePtr)
{
    BOOST_ASSERT(m_storePtr.get() != NULL);
}

//--------------------------------------------------------------------------------------------------
PathIteratorToConfigurationView::PathIteratorToConfigurationView(const PathIteratorToConfigurationView &rhs):
        m_storePtr(rhs.m_storePtr)
{
}

//--------------------------------------------------------------------------------------------------
PathIteratorToConfigurationView &PathIteratorToConfigurationView::operator=(const PathIteratorToConfigurationView &rhs)
{
    if (this != &rhs)
    {
        m_storePtr = rhs.m_storePtr;
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
ConfigurationView PathIteratorToConfigurationView::operator()(const Detail::RealPath &path) const
{
    return ConfigurationView(ConfigurationBase(m_storePtr, path));
}

} /* namespace Configuration */
} /* namespace Tools */

