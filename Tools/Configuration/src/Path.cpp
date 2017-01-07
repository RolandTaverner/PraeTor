#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include "Tools/Configuration/Path.h"

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
Path::Path()
{
}

//--------------------------------------------------------------------------------------------------
Path::Path(const Path &rhs)
{
    *this = rhs;
}

//--------------------------------------------------------------------------------------------------
Path::Path(const std::string &path)
{
    boost::char_separator<char> separator(".", "", boost::keep_empty_tokens);
    tokenizer pathTokens(path, separator);

    tokenizer::const_iterator i = pathTokens.begin();
    while(i != pathTokens.end())
    {
        //TODO: use unicode
        const std::string element = boost::to_lower_copy(boost::trim_copy(*i));
        if (element.empty())
        {
            throw WrongPathError("Path contains empty elements", path);
        }
        m_pathElements.push_back(element);
        ++i;
    }
}

//--------------------------------------------------------------------------------------------------
Path::~Path()
{
}

//--------------------------------------------------------------------------------------------------
Path &Path::operator=(const Path &rhs)
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
bool Path::operator==(const Path &rhs) const
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
Path Path::parent() const
{
    if (isRoot())
    {
        //TODO: throw special exception
        throw std::runtime_error("Path is top-level");
    }

    Path p(*this);
    p.m_pathElements.pop_back();
    return p;
}

//--------------------------------------------------------------------------------------------------
std::string Path::string() const
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
        pathStream << *i;
    }
    return pathStream.str();
}

//--------------------------------------------------------------------------------------------------
Path Path::operator/(const Path &tail) const
{
    Path p(*this);
    std::copy(tail.m_pathElements.begin(),
              tail.m_pathElements.end(),
              std::back_inserter(p.m_pathElements));
    return p;
}

//--------------------------------------------------------------------------------------------------
Path Path::operator/(const std::string &tail) const
{
    const Path tailPath(tail);
    return *this / tailPath;
}

//--------------------------------------------------------------------------------------------------
bool Path::isRoot() const
{
    return m_pathElements.empty();
}

//--------------------------------------------------------------------------------------------------
const Path::PathElements &Path::getElements() const
{
    return m_pathElements;
}

} /* namespace Configuration */
} /* namespace Tools */
