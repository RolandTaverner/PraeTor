#include "Tools/Configuration/ConfigurationBase.h"
#include "Tools/Configuration/Store.h"

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
ConfigurationBase::ConfigurationBase()
{
    m_storePtr.reset(new Detail::Store());
}

//--------------------------------------------------------------------------------------------------
ConfigurationBase::~ConfigurationBase()
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationBase::ConfigurationBase(const ConfigurationBase &rhs) :
        m_storePtr(rhs.m_storePtr), m_branchPath(rhs.m_branchPath)
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationBase::ConfigurationBase(boost::shared_ptr<Detail::Store> storePtr,
                                     const Detail::RealPath &branchPath) :
        m_storePtr(storePtr), m_branchPath(branchPath)
{
}

//--------------------------------------------------------------------------------------------------
ConfigurationBase &ConfigurationBase::operator=(const ConfigurationBase &rhs)
{
    if (this != &rhs)
    {
        m_storePtr = rhs.m_storePtr;
        m_branchPath = rhs.m_branchPath;
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
const Detail::RealPath &ConfigurationBase::getPath() const
{
    return m_branchPath;
}

//--------------------------------------------------------------------------------------------------
Detail::Store &ConfigurationBase::getStore() const
{
    BOOST_ASSERT(m_storePtr.get() != NULL);
    return *m_storePtr;
}

} /* namespace Configuration */
} /* namespace Tools */
