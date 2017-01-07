#ifndef TOOLS_CONFIGURATION_XMLPARSER_H_
#define TOOLS_CONFIGURATION_XMLPARSER_H_

// C++
#include <istream>
#include <ostream>

#include "Tools/Configuration/Configuration.h"

namespace Tools
{
namespace Configuration
{
namespace Parsers
{
namespace Xml
{

Configuration readXml(const std::string &filePath);
Configuration readXml(std::istream &stream);

void writeXml(const ConfigurationView &conf, const std::string &filePath);
void writeXml(const ConfigurationView &conf, std::ostream &stream);

} /* namespace Xml */
} /* namespace Parsers */
} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_XMLPARSER_H_ */
