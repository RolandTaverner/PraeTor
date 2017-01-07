#ifndef COMPRESS__BASE64_H
#define COMPRESS__BASE64_H

// STL
#include <string>

namespace Tools
{

namespace CompressUtils
{

/**
 * Кодирование в Base64
 */

/**
 * @brief Кодирует входную строку с помощью base64(параметры по умолчанию).
 * @param input - входная строка
 * @exception std::exception - если произошла ошибка кодирования
 * @return Возвращает запакованную строку
 */
std::string base64StringEncode(const std::string &input);

/**
 * @brief Кодирует входную строку с помощью base64(параметры по умолчанию).
 * @param input - входная строка
 * @param encodedOutput - выходная строка. Предыдущее содержимое удаляется
 * @exception std::exception - если произошла ошибка кодирования
 */
void base64StringEncode(const std::string &input, std::string &encodedOutput);

/**
 * @brief Кодирует поток с помощью base64(параметры по умолчанию).
 * @param istreamIn - входной поток
 * @param ostreamOut - выходной поток
 * @exception std::exception - если произошла ошибка кодирования
 */
void base64StringEncode(std::istream &istreamIn, std::ostream &ostreamOut);

/**
 * Декодирование из Base64
 */

/**
 * @brief Декодирует входную строку по алгоритму base64 (параметры по умолчанию).
 * @param inputEncoded - входная запакованная строка
 * @exception std::exception - если произошла ошибка распаковки
 * @return Возвращает декодированную строку
 */
std::string base64StringDecode(const std::string &inputEncoded);

/**
 * @brief Декодирует входную строку по алгоритму base64 (параметры по умолчанию).
 * @param inputEncoded - входная строка
 * @param output - выходная строка. Предыдущее содержимое удаляется
 * @exception std::exception - если произошла ошибка кодирования
 */
void base64StringDecode(const std::string &inputEncoded, std::string &output);

/**
 * @brief Декодирует поток с помощью base64 (параметры по умолчанию).
 * @param istreamIn - входной поток
 * @param ostreamOut - выходной поток
 * @exception std::exception - если произошла ошибка декодирования
 */
void base64StringDecode(std::istream &istreamIn, std::ostream &ostreamOut);

} // namespace CompressUtils

} // namespace Tools

#endif // COMPRESS__BASE64_H

