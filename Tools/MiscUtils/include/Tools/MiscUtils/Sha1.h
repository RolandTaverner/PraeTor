#ifndef TOOLS_MISCUTILS_SHA1_H_
#define TOOLS_MISCUTILS_SHA1_H_

#include <boost/uuid/sha1.hpp>

#include "BaseHash.h"

namespace Tools
{
namespace MiscUtils
{

/**
 * Размер хеша SHA1
 */
static const size_t SHA1_SIZE = 20;

/**
 * Значение пустого SHA-1 хеша
 */
static const boost::uint8_t EMPTY_SHA1[] = {
    0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55,
    0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09
};

/**
 * @brief SHA-1 хеш
 *
 * Структура содержит данные хеша SHA-1
 */
struct Sha1: public BaseHash<SHA1_SIZE>
{
public:

    /**
     * Создает объект, очищает данные хеша
     */
    Sha1()
    {
    }

    /**
     * Создает объект, копирует данные хеша из буфера
     * @param buffer - буфер
     */
    Sha1(const DataType buffer)
        : BaseHash<SHA1_SIZE>(buffer)
    {
    }

    /**
     * Создает объект, извлекает данные из boost::uuids::detail::sha1 генератора
     * @param sha1 - генератор sha1
     */
    Sha1(boost::uuids::detail::sha1 sha1)
    {
        boost::uint32_t digest[5];
        sha1.get_digest(digest);
        init(digest);
    }
    /**
     * Создает объект, копирует данные хеша из буфера
     * @param buffer - буфер
     */
    Sha1(const boost::uint32_t buffer[5])
    {
        init(buffer);
    }

    /**
     * @return true - если хеш пустой, иначе false
     */
    inline virtual bool isEmpty() const
    {
        return std::memcmp(m_data, EMPTY_SHA1, size()) == 0;
    }

private:
    /**
     * Инициализирует данные хеша из boost::uint32_t буфера
     * @param buffer - буфер
     */
    inline void init(const boost::uint32_t buffer[5])
    {
        BOOST_ASSERT(buffer);
#if defined(BOOST_LITTLE_ENDIAN)
        // Изменить порядок байтов на big-endian
        boost::uint32_t bigEndianBuffer[5];
        for (size_t i = 0; i < 5; ++i)
        {
            bigEndianBuffer[i] = ((buffer[i] & 0xFF) << 24) |
                                 ((buffer[i] & 0xFF00) << 8) |
                                 ((buffer[i] & 0xFF0000) >> 8) |
                                 ((buffer[i] & 0xFF000000) >> 24);
        }
        std::memcpy(m_data, bigEndianBuffer, size());
#else
        std::memcpy(m_data, buffer, size());
#endif
    }

};

} // namespace MiscUtils
} // namespace Tools

#endif /* TOOLS_MISCUTILS_SHA1_H_ */
