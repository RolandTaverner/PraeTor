#ifndef COMPRESS_H
#define COMPRESS_H

// C++
#include <stdexcept>
#include <string>

// BOOST
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/algorithm/string.hpp>

namespace Tools
{
namespace CompressUtils
{

/**
 * Сжимает входную строку
 * @param input - входная строка
 * @param compressType - тип сжатия (zlib или gzip)
 * @exception std::exception - если произошла ошибка сжатия
 * @return Возвращает сжатую строку
 */
std::string stringCompress(const std::string &input, const std::string &compressType);

/**
 * Разжимает входную строку
 * @param inputCompressed - входная строка
 * @param compressType - тип сжатия (zlib или gzip)
 * @exception std::exception - если произошла ошибка сжатия
 * @return Возвращает расжатую строку
 */
std::string stringUncompress(const std::string &inputCompressed, const std::string &compressType);

/**
 * Сжимает входную строку с помощью zlib (парамерты сжатия по умолчанию).
 * @see zlibStringUncompress
 * @param input - входная строка
 * @exception std::exception - если произошла ошибка сжатия
 * @return Возвращает сжатую строку
 */
std::string zlibStringCompress(const std::string &input);

/**
 * Распаковывает сжатую входную строку с помощью zlib (парамерты сжатия по умолчанию).
 * @see zlibStringCompress
 * @param inputCompressed - входная сжатая строка
 * @exception std::exception - если произошла ошибка расжатия
 * @return Возвращает распакованную строку
 */
std::string zlibStringUncompress(const std::string &inputCompressed);

/**
 * Сжимает входную строку с помощью gzip (парамерты сжатия по умолчанию).
 * @see gzipStringUncompress
 * @param input - входная строка
 * @exception std::exception - если произошла ошибка сжатия
 * @return Возвращает сжатую строку
 */
std::string gzipStringCompress(const std::string &input);

/**
 * Распаковывает сжатую входную строку с помощью gzip (парамерты сжатия по умолчанию).
 * @see gzipStringCompress
 * @param inputCompressed - входная сжатая строка
 * @exception std::exception - если произошла ошибка расжатия
 * @return Возвращает распакованную строку
 */
std::string gzipStringUncompress(const std::string &inputCompressed);

/**
 * @brief Билдер потока чтения (в зависимости от расширения файла .gz .bz2).
 * @param istream - поток чтения
 * @param file - путь к файлу
 */
inline void buildStream(boost::iostreams::filtering_istream & istream, const std::string & file)
{

    if (boost::ends_with(file, ".gz") || boost::ends_with(file, ".bz2"))
    {
        //    file_stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        if (boost::ends_with(file, ".gz"))
        {
            istream.push(boost::iostreams::gzip_decompressor());
        }
        if (boost::ends_with(file, ".bz2"))
        {
            istream.push(boost::iostreams::bzip2_decompressor());
        }
        istream.push(boost::iostreams::file_source(file, std::ios_base::in | std::ios_base::binary));
    }
    else
    {
        istream.push(boost::iostreams::file_source(file));
    }
}

/**
 * @brief Билдер потока записи (в зависимости от расширения файла .gz .bz2).
 * @param ostream - поток записи
 * @param file
 */
inline void buildStream(boost::iostreams::filtering_ostream & ostream, const std::string & file)
{
    if (boost::ends_with(file, ".gz") || boost::ends_with(file, ".bz2"))
    {
        //    file_stream.open(file.c_str(), std::ios_base::out|std::ios_base::binary);
        //    file_stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        if (boost::ends_with(file, ".gz"))
        {
            ostream.push(boost::iostreams::gzip_compressor());
        }
        if (boost::ends_with(file, ".bz2"))
        {
            ostream.push(boost::iostreams::bzip2_compressor());
        }
        ostream.push(boost::iostreams::file_sink(file, std::ios_base::out | std::ios_base::binary));
    }
    else
    {
        ostream.push(boost::iostreams::file_sink(file));
    }
}

} // namespace CompressUtils
} // namespace Tools

#endif /* COMPRESS_H */
