#include "Tools/StringUtils/StringEscapeUtils.h"
#include "Tools/StringUtils/UnicodeTextProcessing.h"

namespace Tools
{
namespace StringUtils
{
namespace EscapeUtils
{

//------------------------------------------------------------------------------
std::string escapeStringToXml(const std::string &input)
{
    std::string output;
    Tools::StringUtils::escapeStringToXml( UnicodeString::fromUTF8(input) ).toUTF8String(output);
    return output;
}

} /* namespace EscapeUtils */
} /* namespace StringUtils */
} /* namespace Tools */
