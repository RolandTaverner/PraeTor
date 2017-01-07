#ifndef STRINGUTILS_STRINGESCAPEUTILS_H_
#define STRINGUTILS_STRINGESCAPEUTILS_H_

#include <string>

namespace Tools
{
namespace StringUtils
{
namespace EscapeUtils
{

/**
 * Экранирует недопустимые (в смысле XML) символы
 * @param input - входная строка
 * @return Возвращает валидную (для записи в XML) строку
 *
 * @code
 * std::string input ="<i>procter & gamble</i>";
 * std::string output = toXml(input); // В результате ouput содержит строку "&lt;i&gt;procter &amp; gamble&lt;/i&gt;"
 * @endcode
 */
std::string escapeStringToXml(const std::string &input);

} /* namespace EscapeUtils */
} /* namespace StringUtils */
} /* namespace Tools */

#endif /* STRINGUTILS_STRINGESCAPEUTILS_H_ */
