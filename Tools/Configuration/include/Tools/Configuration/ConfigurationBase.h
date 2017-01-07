#ifndef INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONBASE_H_
#define INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONBASE_H_

// Boost
#include "boost/shared_ptr.hpp"

#include "Tools/Configuration/Path.h"
#include "Tools/Configuration/RealPath.h"

namespace Tools
{
namespace Configuration
{

namespace Detail
{
    class Store;
}

class ConfigurationBase
{
public:
    ConfigurationBase();
    ConfigurationBase(const ConfigurationBase &rhs);
    ConfigurationBase(boost::shared_ptr<Detail::Store> storePtr, const Detail::RealPath &branchPath);
    virtual ~ConfigurationBase();
    ConfigurationBase &operator=(const ConfigurationBase &rhs);

protected:
    const Detail::RealPath &getPath() const;
    Detail::Store &getStore() const;

protected:
    boost::shared_ptr<Detail::Store> m_storePtr;
    Detail::RealPath m_branchPath;
};

} /* namespace Configuration */
} /* namespace Tools */

#endif /* INCLUDE_TOOLS_CONFIGURATION_CONFIGURATIONBASE_H_ */
