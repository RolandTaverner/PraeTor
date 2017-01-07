#ifndef INCLUDE_TOOLS_STRINGUTILS_JSONUTILS_H
#define INCLUDE_TOOLS_STRINGUTILS_JSONUTILS_H

#include <string>

namespace Tools
{
namespace StringUtils
{

std::string escapeJSON( const std::string &input );

std::string unescapeJSON( const std::string &input );

} // namespace StringUtils
} // namespace Tools

#endif //#ifndef INCLUDE_TOOLS_STRINGUTILS_JSONUTILS_H

