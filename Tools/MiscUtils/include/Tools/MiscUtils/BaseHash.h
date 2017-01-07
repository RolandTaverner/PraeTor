#ifndef TOOLS_MISCUTILS_BASE_HASH_H_
#define TOOLS_MISCUTILS_BASE_HASH_H_

#include <cstring>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/detail/endian.hpp>

namespace Tools
{
namespace MiscUtils
{

template <size_t hashSize>
struct BaseHash;

/**
 * Оператор вывода хеша в поток в виде hex строки
 * @param stream - поток вывода
 * @param hash - хеш
 * @return поток вывода
 */
template <size_t hashSize>
inline std::ostream &operator<< (std::ostream &stream, const BaseHash<hashSize> &hash)
{
    for (size_t i = 0; i < hash.size(); ++i)
    {
        stream << std::setfill('0') << std::setw(2) << std::hex
               << static_cast<boost::uint16_t>(hash.data()[i]);
    }
    return stream;
}

/**
 * @brief Хэш
 *
 * Структура содержит данные хеша без привязки к конкретному алгоритму
 * @tparam hashSize - размер хэша в октетах
 */
template <size_t hashSize>
struct BaseHash
{

public:
    /** Тип для хранения данных хеша */
    typedef boost::uint8_t DataType[hashSize];

    /**
     * Создает объект, очищает данные хеша
     */
    BaseHash()
    {
        clear();
    }

    /**
     * Создает объект, копирует данные хеша из буфера
     * @param buffer - буфер
     */
    BaseHash(const DataType buffer)
    {
        BOOST_ASSERT( buffer );
        std::memcpy( m_data, buffer, size() );
    }

    /**
     * Пустой виртуальный деструктор
     */
    virtual ~BaseHash()
    {}

    /**
     * @return размер данных хеша
     */
    size_t size() const
    {
        return sizeof(m_data);
    }

    /**
     * Копирует данные хеша
     * @param buffer - целевой буфер
     */
    void copy(DataType buffer) const
    {
        BOOST_ASSERT(buffer);
        std::memcpy(buffer, m_data, size());
    }
    /**
     * Очищает данные хеша
     */
    void clear()
    {
        std::memset(m_data, 0, size());
    }

    /**
     * @return true - если хеш не содержит данных, иначе false
     */
    bool isNull() const
    {
        return static_cast<size_t>(std::count(m_data, m_data + size(), 0)) == size();
    }
    /**
     * @return true - если хеш пустой, иначе false
     */
    virtual bool isEmpty() const
    {
        return std::memcmp(m_data, ZERO_HASH, size()) == 0;
    }

    /**
     * @return данные хеша
     */
    const boost::uint8_t *data() const
    {
        return m_data;
    }
    /**
     * @return hex строку данных хеша
     */
    std::string string() const
    {
        std::ostringstream out;
        out << *this;
        return out.str();
    }
    /**
     * @return строку, содержащую "сырые" данные хеша
     */
    std::string rawString() const
    {
        return std::string(reinterpret_cast<const char*>(m_data), size());
    }

/// @name Операторы сравнения данных двух хешей
/// @{
    bool operator < (const BaseHash<hashSize> &right) const
    { return std::memcmp(m_data, right.m_data, size()) < 0; }
    bool operator > (const BaseHash<hashSize> &right) const
    { return std::memcmp(m_data, right.m_data, size()) > 0; }
    bool operator == (const BaseHash<hashSize> &right) const
    { return std::memcmp(m_data, right.m_data, size()) == 0; }
    bool operator != (const BaseHash<hashSize> &right) const
    { return std::memcmp(m_data, right.m_data, size()) != 0; }
/// @}

protected:

    DataType m_data; //!< Данные хеша

private:

    static const boost::uint8_t ZERO_HASH[hashSize];

};

template <size_t hashSize>
const boost::uint8_t BaseHash<hashSize>::ZERO_HASH[hashSize] = {0};

} // namespace MiscUtils
} // namespace Tools

#endif //TOOLS_MISCUTILS_BASE_HASH_H_
