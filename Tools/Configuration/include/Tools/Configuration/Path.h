#ifndef TOOLS_CONFIGURATION_PATH_H_
#define TOOLS_CONFIGURATION_PATH_H_

#include <list>
#include <string>

#include "Tools/Configuration/Errors.h"

namespace Tools
{
namespace Configuration
{

class Path
{
public:
    typedef std::string PathElement;
    typedef std::list<PathElement> PathElements;

    Path();
    Path(const Path &rhs);
    explicit Path(const std::string &path);
    virtual ~Path();

    Path &operator=(const Path &rhs);
    bool operator==(const Path &rhs) const;

    Path parent() const;
    std::string string() const;

    Path operator/(const std::string &tail) const;
    Path operator/(const Path &tail) const;

    bool isRoot() const;
    const PathElements &getElements() const;

private:
    PathElements m_pathElements;
};

} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_PATH_H_ */
