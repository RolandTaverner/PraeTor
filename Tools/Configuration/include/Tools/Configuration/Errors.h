#ifndef TOOLS_CONFIGURATION_ERRORS_H_
#define TOOLS_CONFIGURATION_ERRORS_H_

#include <stdexcept>

namespace Tools
{
namespace Configuration
{

//--------------------------------------------------------------------------------------------------
class ConfigError : public std::runtime_error
{
public:
    explicit ConfigError(const std::string &message) :
            std::runtime_error(message)
    {
    }

    virtual ~ConfigError() 
    {
    }
};

//--------------------------------------------------------------------------------------------------
class PathError : public ConfigError
{
public:
    explicit PathError(const std::string &message, const std::string &path) :
            ConfigError(message), m_path(path)
    {
    }

    virtual ~PathError() 
    {
    }

    const std::string &getPath() const
    {
        return m_path;
    }

private:
    std::string m_path;
};

class WrongPathError : public PathError
{
public:
    WrongPathError(const std::string &message, const std::string &path) :
            PathError(message, path)
    {
    }

    virtual ~WrongPathError() 
    {
    }
};

//--------------------------------------------------------------------------------------------------
class ResolveError : public ConfigError
{
public:
    explicit ResolveError(const std::string &message, const std::string &path) :
            ConfigError(message), m_path(path)
    {
    }

    virtual ~ResolveError() 
    {
    }

    const std::string &getPath() const
    {
        return m_path;
    }

private:
    std::string m_path;
};

class AmbiguousPathError : public ResolveError
{
public:
    AmbiguousPathError(const std::string &message, const std::string &path) :
            ResolveError(message, path)
    {
    }

    virtual ~AmbiguousPathError() 
    {
    }
};

class StoreChangedError : public ResolveError
{
public:
    StoreChangedError(const std::string &message, const std::string &path) :
            ResolveError(message, path)
    {
    }

    virtual ~StoreChangedError() 
    {
    }
};

class PathNotExistsError : public ResolveError
{
public:
    PathNotExistsError(const std::string &message, const std::string &path) :
            ResolveError(message, path)
    {
    }

    virtual ~PathNotExistsError() 
    {
    }
};

//--------------------------------------------------------------------------------------------------
class MergeError : public ConfigError
{
public:
    explicit MergeError(const std::string &message) :
            ConfigError(message)
    {
    }

    virtual ~MergeError() 
    {
    }
};

//--------------------------------------------------------------------------------------------------
class RemoveError : public ConfigError
{
public:
    explicit RemoveError(const std::string &message) :
            ConfigError(message)
    {
    }

    virtual ~RemoveError() 
    {
    }
};

//--------------------------------------------------------------------------------------------------
class AttrNotFoundError : public ConfigError
{
public:
    explicit AttrNotFoundError(const std::string &message, const std::string &path, const std::string &attrName) :
            ConfigError(message), m_path(path), m_attrName(attrName)
    {
    }

    virtual ~AttrNotFoundError() 
    {
    }

    const std::string &getPath() const
    {
        return m_path;
    }

private:
    std::string m_path;
    std::string m_attrName;
};

} /* namespace Configuration */
} /* namespace Tools */

#endif /* TOOLS_CONFIGURATION_ERRORS_H_ */
