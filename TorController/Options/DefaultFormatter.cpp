#include <boost/algorithm/string/replace.hpp>
#include <boost/assert.hpp>
#include <boost/variant/static_visitor.hpp>

#include "DefaultFormatter.h"

//-------------------------------------------------------------------------------------------------
DefaultFormatter::DefaultFormatter()
{
}

//-------------------------------------------------------------------------------------------------
DefaultFormatter::~DefaultFormatter()
{
}

//-------------------------------------------------------------------------------------------------
class OptionValueToStringVisitor
    : public boost::static_visitor<>
{
public:
    OptionValueToStringVisitor()
    {
    }

    void operator()(const OptionSingleValue &v)
    {
        m_value = v;
    }

    void operator()(const OptionListValue &v)
    {
        for (const OptionSingleValue &s : v)
        {
            if (!m_value.empty())
            {
                m_value += ",";
            }
            m_value += s;
        }
    }

    const std::string &getValue() const
    {
        return m_value;
    }

private:
    std::string m_value;
};

// TODO: implement this
void DefaultFormatter::format(const Option &option, 
                              const OptionDesc &desc,
                              std::ostream &out) const
{
    BOOST_ASSERT(option.value().is_initialized());

    std::string format = desc.get<8>().get("");
    boost::algorithm::replace_all(format, "%NAME%", option.name());

    OptionValueToStringVisitor v;
    boost::apply_visitor(v, option.value().get());
    boost::algorithm::replace_all(format, "%VALUE%", v.getValue());

    out << format;
}