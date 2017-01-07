// Original code was provided by mariolpantunes
// http://stackoverflow.com/users/1236117/mariolpantunes

// C++
#include <sstream>
#include <cctype>
#include <iomanip> // setfill

#include "Tools/StringUtils/JSONUtils.h"

namespace Tools
{
namespace StringUtils
{

enum State { ESCAPED, UNESCAPED };

std::string escapeJSON( const std::string& input )
{
  std::string output;
  output.reserve( input.length() );

  for ( std::string::size_type i = 0; i < input.length(); ++i )
  {
    switch (input[i]) 
    {
      case '"':
          output += "\\\"";
          break;
      case '/':
          output += "\\/";
          break;
      case '\b':
          output += "\\b";
          break;
      case '\f':
          output += "\\f";
          break;
      case '\n':
          output += "\\n";
          break;
      case '\r':
          output += "\\r";
          break;
      case '\t':
          output += "\\t";
          break;
      case '\\':
          output += "\\\\";
          break;
      default:
          /** @see jsoncpp, json_tool.h */
          std::string::const_reference ch = input[i];
          if (std::iscntrl(ch))
          {
              std::ostringstream oss;
              oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
                  << static_cast<int>(ch);
              output += oss.str();
          }
          else
          {
              output += ch;
          }
          break;
    }
  }

  return output;
}

std::string unescapeJSON( const std::string& input )
{
  State s = UNESCAPED;
  std::string output;
  output.reserve(input.length());

  for (std::string::size_type i = 0; i < input.length(); ++i)
  {
    switch(s)
    {
      case ESCAPED:
        {
          switch( input[i] )
          {
            case '"':
                output += '\"';
                break;
            case '/':
                output += '/';
                break;
            case 'b':
                output += '\b';
                break;
            case 'f':
                output += '\f';
                break;
            case 'n':
                output += '\n';
                break;
            case 'r':
                output += '\r';
                break;
            case 't':
                output += '\t';
                break;
            case '\\':
                output += '\\';
                break;
            default:
                output += input[i];
                break;
          }

          s = UNESCAPED;
          break;
        }
    case UNESCAPED:
        {
          switch( input[i] )
          {
            case '\\':
              s = ESCAPED;
              break;
            default:
              output += input[i];
              break;
          }
        }
    }
  }
  return output;
}

} // namespace StringUtils
} // namespace Tools
