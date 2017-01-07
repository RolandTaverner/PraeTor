#include <string.h>

// STL
#include <string>
#include <iostream>
#include <sstream>

// b64
#include "Tools/CompressUtils/b64/decode.h"
#include "Tools/CompressUtils/b64/encode.h"

#include "Tools/CompressUtils/Base64.h"

namespace Tools
{

    namespace CompressUtils
    {

    using namespace std;

    /**
     * Кодирует входную строку с помощью base64(парамерты по умолчанию).
     * @see base64StringEncode
     * @param input - входная строка
     * @exception std::exception - если произошла ошибка сжатия
     * @return Возвращает запакованную строку
     */
    std::string base64StringEncode(const std::string &input)
    {
        stringstream inputStrm(input);
        stringstream outputStrm;

        base64StringEncode(inputStrm, outputStrm);
        return outputStrm.str();
    }

    /**
     * Кодирует входную строку с помощью base64(парамерты по умолчанию).
     * @see base64StringEncode
     * @param input - входная строка
     * @param encodedOutput - выходная строка. Предыдущее содержимое удаляется
     * @exception std::exception - если произошла ошибка кодирования
     */
    void base64StringEncode(const std::string &input, std::string &encodedOutput)
    {
        stringstream inputStrm(input);
        stringstream outputStrm;

        base64StringEncode(inputStrm, outputStrm);
        encodedOutput = outputStrm.str();
    }

    /**
     * Кодирует поток с помощью base64(парамерты по умолчанию).
     * @see base64StringEncode
     * @param istreamIn - входной поток
     * @param ostreamOut - выходной поток
     * @exception std::exception - если произошла ошибка кодирования
     */
    void base64StringEncode(std::istream &istreamIn, std::ostream &ostreamOut)
    {
        base64::encoder encoder;
        encoder.encode(istreamIn, ostreamOut);
    }

//=============================================================================

    /**
     * Распаковывает сжатую входную строку с помощью base64 (парамерты по умолчанию).
     * @see base64StringDecode
     * @param inputEncoded - входная запакованная строка
     * @exception std::exception - если произошла ошибка распаковки
     * @return Возвращает распакованную строку
     */
    std::string base64StringDecode(const std::string &inputEncoded)
    {
        stringstream inputStrm(inputEncoded);
        stringstream outputStrm;

        base64StringDecode(inputStrm, outputStrm);
        return outputStrm.str();
    }

    /**
     * Декодирует входную строку по алгоритму base64 (парамерты по умолчанию).
     * @see base64StringEncode
     * @param inputEncoded - входная строка
     * @param output - выходная строка. Предыдущее содержимое удаляется
     * @exception std::exception - если произошла ошибка кодирования
     */
    void base64StringDecode(const std::string &inputEncoded, std::string &output)
    {
        stringstream inputStrm(inputEncoded);
        stringstream outputStrm;

        base64StringDecode(inputStrm, outputStrm);

        output = outputStrm.str();
    }

    /**
     * Декодирует поток с помощью base64 (парамерты по умолчанию).
     * @see base64StringDecode
     * @param istreamIn - входной поток
     * @param ostreamOut - выходной поток
     * @exception std::exception - если произошла ошибка декодирования
     */
    void base64StringDecode(std::istream &istreamIn, std::ostream &ostreamOut)
    {
        base64::decoder decoder;
        decoder.decode(istreamIn, ostreamOut);
    }

    } // namespace CompressUtils

} // namespace Tools
