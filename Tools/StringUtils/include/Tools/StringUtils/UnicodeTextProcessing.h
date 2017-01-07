#ifndef UNICODETEXTPROCESSING_H
#define UNICODETEXTPROCESSING_H

#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>

#include <unicode/bytestream.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/utypes.h>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>

namespace Tools
{
namespace StringUtils
{

/**
 * Разбивает входную строку на части, используя в качестве разделителя <i>delimeter</i>
 * @param input - входная строка
 * @param results - выходной вектор
 * @param delimeter - символ-разделитель
 *
 * @code
 * UnicodeString input = UnicodeString::fromUtf8("Мама мыла раму");
 * std::vector<UnicodeString> parts;
 * split(input, parts, L' ');  // В результате parts содержит {"Мама", "мыла", "раму"}
 * @endcode
 */
void split(const UnicodeString &input, std::vector<UnicodeString> &results, UChar delimeter);

/**
 * Удалет начальные, конечные и дублирующиеся пробельные символы в строке
 * @param input - входная строка
 * @param spaceOnly - признак замены пробельных символов (перевод строки, возврат корретки и т.п.) на пробелы
 * @return Возвращает исправленную копию строки
 *
 * @code
 * UnicodeString input = UnicodeString::fromUtf8(" Мама  мыла раму   ");
 * UnicodeString output = trimDuplicateSpace(input);  // В результате output содержит "Мама мыла раму"
 * @endcode
 */
UnicodeString trimDuplicateSpace(const UnicodeString &input, bool spaceOnly = true);

/**
 * Нормализует входную строку "в смысле поиска"
 * @param input - входная строка
 * @param upper - признак перевода символов в верхний регистр (иначе в нижний регистр)
 * @param spaceOnly - признак замены пробельных символов (перевод строки, возврат корретки и т.п.) на пробелы
 * @param changeTable - таблица с правилами замены символов
 * @return Возвращает исправленную строку
 *
 * @code
 * UnicodeString input = UnicodeString::fromUTF8("Медве́жьи (лат. Ursidae) — семейство млекопитающих отряда хищных. Отличаются от других представителей псообразных более коренастым телосложением.");
 * Unicode output = normalizeTextQuery(input); // В результате output содержит строку "МЕДВЕЖЬИ ЛАТ URSIDAE СЕМЕЙСТВО МЛЕКОПИТАЮЩИХ ОТРЯДА ХИЩНЫХ ОТЛИЧАЮТСЯ ОТ ДРУГИХ ПРЕДСТАВИТЕЛЕЙ ПСООБРАЗНЫХ БОЛЕЕ КОРЕНАСТЫМ ТЕЛОСЛОЖЕНИЕМ"
 * @endcode
 */
UnicodeString normalizeTextQuery(const UnicodeString &input,
                                 bool upper = true,
                                 bool spaceOnly = true,
                                 const std::vector<std::pair<UChar32, UChar32> > &changeTable = std::vector<std::pair<UChar32, UChar32> >());

/**
 * Экранирует недопустимые (в смысле XML) символы
 * @param input - входная строка
 * @return Возвращает валидную (для записи в XML) строку
 *
 * @code
 * UnicodeString input = UnicodeString::fromUTF8("<i>procter & gamble</i>");
 * UnicodeString output = toXml(input); // В результате ouput содержит строку "&lt;i&gt;procter &amp; gamble&lt;/i&gt;"
 * @endcode
 */
UnicodeString escapeStringToXml(const UnicodeString &input);

/**
 * "Меняет" раскладку клавиатуры
 * @param input - входная строка
 * @return Возвращает исправленную строку
 *
 * @code
 * UnicodeString input = UnicodeString::fromUTF8("Ghbdtn");
 * UnicodeString output = puntoSwitcher(input); // В результате output содержит строку "Привет"
 * @endcode
 */
UnicodeString puntoSwitcher(const UnicodeString &input);

/**
 * Конвертирует строку в UnicodeString
 * @param input - входная строка
 * @param encoding - кодировка входной строки
 * @return UnicodeString
 */
UnicodeString convertToUnicodeString(const std::string &input, const std::string &encoding);

/**
 * Конвертирует строку UnicodeString в указанную кодировку
 * @param input - входная строка
 * @param encoding - кодировка входной строки
 * @return std::string
 */
std::string convertFromUnicodeString(const UnicodeString &input, const std::string &encoding);

/**
 * @brief Потокобезопасный класс для кэширования конвертеров.
 */
class UnicodeConverters
{
public:
    typedef boost::shared_ptr<UConverter> ConverterPtr;

    /**
     * Получает конвертер из кэша по имени. Если такого конвертера не существует, он будет создан.
     * @param encoding - кодировка
     * @return ConverterPtr - конвертер
     */
    static ConverterPtr getConverter(const std::string &encoding);

private:
    UnicodeConverters();
    ~UnicodeConverters();

private:
    static boost::shared_mutex m_Mutex;
    typedef std::map<std::string, ConverterPtr> ConvertersMap;
    static ConvertersMap m_converters;

    class ConverterDeleter
    {
    public:
        void operator()(UConverter *pConverter);
    };

};

} // namespace StringUtils
} // namespace Tools
#endif // UNICODETEXTPROCESSING_H
