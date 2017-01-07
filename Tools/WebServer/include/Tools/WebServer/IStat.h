#ifndef ISTAT_H_
#define ISTAT_H_

// C++
#include <string>

// BOOST
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace Tools
{
namespace WebServer
{

class IStat
{
public:
    typedef std::pair<std::string, std::string> Parameter;
    typedef std::vector<Parameter> Parameters;
    typedef boost::function<void (Parameters&)> ParametersProvider;

public:
    virtual ~IStat() {}

    virtual void registerParameter(const std::string &name) = 0;
    virtual void set(const std::string &name, const long value) = 0;
    virtual void increment(const std::string &name, const long value) = 0;
    virtual void registerParametersProvider(const std::string &name,
    		                                const ParametersProvider &parametersProvider) = 0;
    virtual void unregisterParametersProvider(const std::string &name) = 0;
};

typedef boost::shared_ptr<IStat> IStatPtr;

} /* namespace WebServer */
} /* namespace Tools */

#endif /* ISTAT_H_ */
