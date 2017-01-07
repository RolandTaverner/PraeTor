#include <sstream>

#include "Tools/Configuration/RealPath.h"

namespace Tools
{
namespace Configuration
{
namespace Detail
{

//--------------------------------------------------------------------------------------------------
RealPath::RealPath()
{
}

//--------------------------------------------------------------------------------------------------
RealPath::RealPath(const RealPath &rhs)
{
    *this = rhs;
}

//--------------------------------------------------------------------------------------------------
RealPath::~RealPath()
{
}

//--------------------------------------------------------------------------------------------------
RealPath &RealPath::operator=(const RealPath &rhs)
{
    if (this != &rhs)
    {
        m_pathElements.clear();
        if (!rhs.m_pathElements.empty())
        {
            m_pathElements.assign(rhs.m_pathElements.begin(), rhs.m_pathElements.end());
        }
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
bool RealPath::operator==(const RealPath &rhs) const
{
    if (m_pathElements.size() != rhs.m_pathElements.size())
    {
        return false;
    }
    PathElements::const_iterator i = m_pathElements.begin();
    PathElements::const_iterator j = rhs.m_pathElements.begin();

    for (; i != m_pathElements.end(); ++i, ++j)
    {
        if (*i != *j)
        {
            return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
RealPath RealPath::operator/(const RealPath &tail) const
{
    RealPath p(*this);
    std::copy(tail.m_pathElements.begin(),
              tail.m_pathElements.end(),
              std::back_inserter(p.m_pathElements));
    return p;
}

//--------------------------------------------------------------------------------------------------
RealPath RealPath::operator/(const PathElement &tail) const
{
    RealPath p(*this);
    p.m_pathElements.push_back(tail);
    return p;
}

//--------------------------------------------------------------------------------------------------
RealPath RealPath::parent() const
{
    if (isRoot())
    {
        throw PathError("Can't get parent of top-level path", "");
    }

    RealPath p(*this);
    p.m_pathElements.pop_back();
    return p;
}

//--------------------------------------------------------------------------------------------------
bool RealPath::isRoot() const
{
    return m_pathElements.empty();
}

//--------------------------------------------------------------------------------------------------
const RealPath::PathElements &RealPath::getElements() const
{
    return m_pathElements;
}

//--------------------------------------------------------------------------------------------------
std::string RealPath::string() const
{
    std::ostringstream pathStream;
    for (PathElements::const_iterator i = m_pathElements.begin();
            i != m_pathElements.end();
            ++i)
    {
        if (i != m_pathElements.begin())
        {
            pathStream << ".";
        }
        pathStream << i->first << ":" << i->second;
    }
    return pathStream.str();
}

//--------------------------------------------------------------------------------------------------
Path RealPath::toPath() const
{
    Path path;
    for (PathElements::const_iterator i = m_pathElements.begin();
            i != m_pathElements.end();
            ++i)
    {
        path = path / i->first;
    }
    return path;
}

} /* namespace Detail */
} /* namespace Configuration */
} /* namespace Tools */
