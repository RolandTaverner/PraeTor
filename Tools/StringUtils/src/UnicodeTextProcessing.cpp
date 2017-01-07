#include <iostream>

// BOOST
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/lock_types.hpp>

// ICU
#include <unicode/ucnv.h>
#include <unicode/chariter.h>
#include <unicode/schriter.h>

// THIS
#include "Tools/StringUtils/UnicodeTextProcessing.h"

namespace Tools
{
namespace StringUtils
{

//------------------------------------------------------------------------------
void split(const UnicodeString &input, std::vector<UnicodeString> &results, UChar delimeter)
{
    boost::int32_t startPosition = 0;
    boost::int32_t position = 0;
    boost::int32_t length = input.length();

    while ((position = input.indexOf(delimeter, startPosition)) != -1 && startPosition < length)
    {
        UnicodeString target;
        input.extract(startPosition, position - startPosition, target); // FIXME (в версии 4.4 появляется функция tempSubString)
        results.push_back(target);
        startPosition = position + 1;
    }

    if (startPosition < length)
    {
        UnicodeString target;
        input.extract(startPosition, length - startPosition, target); // FIXME (в версии 4.4 появляется функция tempSubString)
        results.push_back(target);
    }
}

//------------------------------------------------------------------------------
UnicodeString trimDuplicateSpace(const UnicodeString &input, bool spaceOnly)
{
	UnicodeString output;
	bool isNeedSpace = false;

	StringCharacterIterator it(input);
	while (it.hasNext())
	{
		UChar32 c = it.current32();

		bool isSpace = (UChar32(0xA0) == c ||
						u_iscntrl(c) ||
						u_isWhitespace(c) ||
						!U_IS_UNICODE_CHAR(c) ||
						!u_isdefined(c) ||
						U_OTHER_SYMBOL == u_charType(c));

		// Если это пробельный символ
		if (isSpace)
		{
			isNeedSpace = !output.isEmpty();
		}
		// Если это непробельный символ
		else
		{
			if (isNeedSpace)
			{
				output.append(L' ');
				isNeedSpace = false;
			}

			output.append(c);
		}

		it.next32();
	}

	return output;
}

//------------------------------------------------------------------------------
UnicodeString normalizeTextQuery(const UnicodeString &input,
                                 bool upper,
                                 bool spaceOnly,
                                 const std::vector<std::pair<UChar32, UChar32> > &changeTable)
{
    // FIXME посмотреть что такое ICU::Normalizer

	UnicodeString output;
	bool isNeedSpace = false;
	size_t changeTableSize = changeTable.size();
	UChar32 p = L' ';

	StringCharacterIterator it(input);
	while (it.hasNext())
	{
		UChar32 c = it.current32();

        // Замена спецсимволов
		bool isChanged = false;
        if (changeTableSize > 0)
        {
            for (size_t j = 0; j < changeTableSize; ++j)
            {
                if (changeTable[j].first == c)
                {
                    c = changeTable[j].second;
                    isChanged = true;
                    break;
                }
            }
        }

        // Заменяем Ё на Е
        if (!isChanged && (c == L'Ё' || c == L'ё'))
        {
            c = L'Е';
        }

        // Меняем регистр
        if (!isChanged)
        {
        	c = (upper ? u_toupper(c) : u_tolower(c));
        }

        // Ударения вырезаем
        if (!isChanged && c == 0x0301)
        {
        	it.next32();
        	continue;
        }

        // Пробел
		bool isSpace = (UChar32(0xA0) == c ||
						u_iscntrl(c) ||
						u_isWhitespace(c) ||
						!U_IS_UNICODE_CHAR(c) ||
						!u_isdefined(c) ||
						U_OTHER_SYMBOL == u_charType(c));

        // Все, что не буква и не цифра заменяем на пробел
        if (!isChanged && !u_isalnum(c))
        {
            isSpace = true;
        }

		// Если это пробельный символ
		if (isSpace)
		{
			isNeedSpace = !output.isEmpty();
		}
		// Если это непробельный символ
		else
		{
			if (isNeedSpace)
			{
				if (!spaceOnly && (u_isWhitespace(p) || u_iscntrl(p)))
				{
					output.append(p);
				}
				else
				{
					output.append(L' ');
				}
				isNeedSpace = false;
			}

			output.append(c);
		}

		p = c;
		it.next32();
	}

	return output;
}

//------------------------------------------------------------------------------
UnicodeString escapeStringToXml(const UnicodeString &input)
{
    boost::int32_t len = input.length();
    UnicodeString output;
    output.releaseBuffer(len);

    static UnicodeString amp  = "&amp;";
    static UnicodeString lt   = "&lt;";
    static UnicodeString gt   = "&gt;";
    static UnicodeString apos = "&apos;";
    static UnicodeString quot = "&quot;";

	StringCharacterIterator it(input);
	while (it.hasNext())
	{
		UChar32 c = it.current32();

		if (U_IS_UNICODE_CHAR(c))
		{
	        switch (c)
	        {
	            case L'&':
	                output.append(amp);
	                break;
	            case L'<':
	                output.append(lt);
	                break;
	            case L'>':
	                output.append(gt);
	                break;
	            case L'\'':
	                output.append(apos);
	                break;
	            case L'"':
	                output.append(quot);
	                break;
	            default:
	                output.append(c);
	                break;
	        }
		}

		it.next32();
	}

	return output;
}

//------------------------------------------------------------------------------
UnicodeString puntoSwitcher(const UnicodeString &input)
{
    static UnicodeString ru = UnicodeString::fromUTF8("ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁйцукенгшщзхъфывапролджэячсмитьбюё");
    static UnicodeString en = UnicodeString::fromUTF8("QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>~qwertyuiop[]asdfghjkl;'zxcvbnm,.`");
    if (ru.length() != en.length())
    {
        return input;
    }

    UnicodeString output;
	StringCharacterIterator it(input);
	while (it.hasNext())
	{
		UChar32 c = it.current32();

		if (U_IS_UNICODE_CHAR(c))
		{
	        boost::int32_t pos = ru.indexOf(c);
	        if (pos != -1)
	        {
	            c = en.char32At(pos);
	        }
	        else
	        {
	            pos = en.indexOf(c);
	            if (pos != -1)
	            {
	                c = ru.char32At(pos);
	            }
	        }

	        output.append(c);
		}

		it.next32();
	}

    return output;
}

//------------------------------------------------------------------------------
UnicodeString convertToUnicodeString(const std::string &input, const std::string &encoding)
{
    UnicodeConverters::ConverterPtr converterPtr = UnicodeConverters::getConverter(encoding);
    if (!converterPtr)
    {
        throw std::invalid_argument("encoding " + encoding + " not found by ICU");
    }

    const int targetCapacity = input.length()*2;
    boost::scoped_array<UChar> ucharsArray(new UChar[targetCapacity]);
    UErrorCode err = U_ZERO_ERROR;

    int targetSize = ucnv_toUChars(converterPtr.get(), ucharsArray.get(), targetCapacity, input.c_str(), input.length(), &err);

    if (err == U_BUFFER_OVERFLOW_ERROR)
    {
        err = U_ZERO_ERROR;
        ucharsArray.reset(new UChar[targetSize]);

        targetSize = ucnv_toUChars(converterPtr.get(), ucharsArray.get(), targetSize, input.c_str(), input.length(), &err);
        if(U_FAILURE(err))
        {
            throw std::runtime_error("string conversion failed (ucnv_toUChars() returned " + boost::lexical_cast<std::string>(err));
        }
    }

    UnicodeString uniString(ucharsArray.get(), targetSize);
    return uniString;
}

//------------------------------------------------------------------------------
std::string convertFromUnicodeString(const UnicodeString &input, const std::string &encoding)
{
    UnicodeConverters::ConverterPtr converterPtr = UnicodeConverters::getConverter(encoding);
    if (!converterPtr)
    {
        throw std::invalid_argument("encoding " + encoding + " not found by ICU");
    }

    const int targetCapacity = input.length();
    boost::scoped_array<char> charsArray(new char[targetCapacity]);
    UErrorCode err = U_ZERO_ERROR;

    int targetSize = ucnv_fromUChars(converterPtr.get(), charsArray.get(), targetCapacity, input.getBuffer(), input.length(), &err);

    if (err == U_BUFFER_OVERFLOW_ERROR)
    {
        err = U_ZERO_ERROR;
        charsArray.reset(new char[targetSize]);

        targetSize = ucnv_fromUChars(converterPtr.get(), charsArray.get(), targetSize, input.getBuffer(), input.length(), &err);
        if(U_FAILURE(err))
        {
            throw std::runtime_error("string conversion failed (ucnv_fromUChars() returned " + boost::lexical_cast<std::string>(err));
        }
    }

    std::string result;
    result.assign(charsArray.get(), targetSize);
    return result;
}

//------------------------------------------------------------------------------

boost::shared_mutex UnicodeConverters::m_Mutex;
UnicodeConverters::ConvertersMap UnicodeConverters::m_converters;

UnicodeConverters::ConverterPtr UnicodeConverters::getConverter(const std::string &encoding)
{
    {
        boost::shared_lock<boost::shared_mutex> sharedLock(m_Mutex);

        ConvertersMap::const_iterator it = m_converters.find(encoding);
        if (it != m_converters.end())
        {
            return it->second;
        }
    }

    boost::upgrade_lock<boost::shared_mutex> upgradeLock(m_Mutex);

    ConvertersMap::const_iterator it = m_converters.find(encoding);
    if (it == m_converters.end())
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(upgradeLock);

        UErrorCode err = U_ZERO_ERROR;
        ConverterPtr pConverter(ucnv_open(encoding.c_str(), &err), ConverterDeleter());
        if (err != U_ZERO_ERROR)
        {
            return ConverterPtr();
        }
        m_converters[encoding] = pConverter;
        return pConverter;
    }

    return it->second;
}

//------------------------------------------------------------------------------
void UnicodeConverters::ConverterDeleter::operator()(UConverter *pConverter)
{
    if (pConverter != NULL)
    {
        ucnv_close(pConverter);
    }
}

//------------------------------------------------------------------------------
} // namespace StringUtils
} // namespace Tools
