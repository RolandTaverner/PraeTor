#pragma once

#include <type_traits>
#include <stdexcept>
#include <string>
#include <system_error>

#include <boost/assert.hpp>
#include <boost/system/error_code.hpp>


class ErrorCode
{
private:
    template<typename T>
    struct dependent_false : std::false_type {};

public:
    ErrorCode()
    {
    }

    ErrorCode(const std::error_code &e, const std::string &message = std::string()) :
        m_stdError(e), m_message(message)
    {
    }

    ErrorCode(const boost::system::error_code &e, const std::string &message = std::string()) :
        m_boostError(e), m_message(message)
    {
    }

    virtual ~ErrorCode() {}

    template<typename T> bool is() const
    {
        static_assert(dependent_false<Type>::value, "Only specializations of is<>() may be used");
    }

    template<> bool is<std::error_code>() const
    {
        return isStdError();
    }

    template<> bool is<boost::system::error_code>() const
    {
        return isBoostError();
    }

    template<typename T> const T &as() const
    {
        static_assert(dependent_false<Type>::value, "Only specializations of as<>() may be used");
    }

    template<> const std::error_code &as<std::error_code>() const
    {
        return m_stdError;
    }

    template<> const boost::system::error_code &as<boost::system::error_code>() const 
    {
        return m_boostError;
    }

    bool isStdError() const noexcept
    {
        return m_stdError.value() != 0;
    }

    bool isBoostError() const noexcept
    {
        return m_boostError;
    }

    typedef void(*unspecified_bool_type)();
    static void unspecified_bool_true() {}

    operator unspecified_bool_type() const noexcept // true if error
    {
        return (isStdError() || isBoostError()) ? unspecified_bool_true : 0;
    }

    bool operator!() const // true if no error
    {
        return !(isStdError() || isBoostError());
    }

    std::string message() const
    {
        if (isBoostError())
        {
            return m_message.empty() ? m_boostError.message() : (m_boostError.message() + ": " + m_message);
        }
        else if (isStdError())
        {
            return m_message.empty() ? m_stdError.message() : (m_stdError.message() + ": " + m_message);
        }
        return std::string();
    }

    std::string category() const
    {
        if (isBoostError())
        {
            return std::string("boost.") + m_boostError.category().name();
        }
        else if (isStdError())
        {
            return m_stdError.category().name();
        }
        return std::string();
    }

    int value() const noexcept
    {
        if (isBoostError())
        {
            return m_boostError.value();
        }
        else if (isStdError())
        {
            return m_stdError.value();
        }
        return 0;
    }

private:
    std::error_code m_stdError;
    boost::system::error_code m_boostError;
    std::string m_message;
};
