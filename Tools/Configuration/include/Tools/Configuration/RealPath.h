#ifndef TOOLS_CONFIGURATION_REALPATH_H_
#define TOOLS_CONFIGURATION_REALPATH_H_

#include <list>
#include <stdexcept>
#include <string>
#include <utility>

#include "Tools/Configuration/Errors.h"
#include "Tools/Configuration/Path.h"

namespace Tools
{
namespace Configuration
{
namespace Detail
{

class RealPath
{
public:
    typedef std::pair<std::string, std::string> PathElement;
    typedef std::list<PathElement> PathElements;

    RealPath();
    RealPath(const RealPath &rhs);
    virtual ~RealPath();

    RealPath &operator=(const RealPath &rhs);
    bool operator==(const RealPath &rhs) const;
    RealPath operator/(const RealPath &tail) const;
    RealPath operator/(const PathElement &tail) const;

    RealPath parent() const;
    bool isRoot() const;
    std::string string() const;

    const PathElements &getElements() const;
    Path toPath() const;
private:
    PathElements m_pathElements;
};

} /* namespace Detail */
} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_REALPATH_H_ */
