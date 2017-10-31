// BOOST
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/foreach.hpp>

// PION
#include <pion/algorithm.hpp>
#include <pion/http/response_writer.hpp>
#include <pion/http/types.hpp>

#include "Tools/CompressUtils/Compress.h"
#include "Tools/StringUtils/StringEscapeUtils.h"
#include "Tools/StringUtils/UnicodeTextProcessing.h"
#include "Tools/WebServer/StatService.h"

#ifdef __GNUC__
#define atomic_inc(ptr, value) __sync_fetch_and_add ((ptr), (value))
#elif defined (_WIN32)
#define atomic_inc(ptr, value) InterlockedExchangeAdd ((ptr), (value))
#else
#error "Need some more porting work here"
#endif

using namespace Tools::StringUtils;

namespace Tools
{
namespace WebServer
{

//--------------------------------------------------------------------------------------------------
StatService::StatService(const std::string &serviceName,
                         const std::string &resource,
                         const std::string &version,
                         const std::string &revision):
                m_isRunning(false),
                m_serviceName(serviceName),
                m_resource(resource),
                m_serviceVersion(version),
                m_serviceRevision(revision)
{
}

//--------------------------------------------------------------------------------------------------
StatService::~StatService()
{
}

//--------------------------------------------------------------------------------------------------
void StatService::start()
{
    m_parameterValues.resize(m_parameterIDs.size(), 0u);
    m_startTime = boost::posix_time::microsec_clock::universal_time();
    m_isRunning = true;
}

//--------------------------------------------------------------------------------------------------
void StatService::stop()
{
    m_isRunning = false;
}

//--------------------------------------------------------------------------------------------------
void StatService::operator()(ConnectionContextPtr contextPtr)
{
    pion::http::request_ptr requestPtr = contextPtr->getRequest();

    const std::string method = requestPtr->get_method();
    const std::string url = requestPtr->get_resource();
    const pion::ihash_multimap &params = requestPtr->get_queries();

    const bool isGzipEnabled = (requestPtr->get_header("Accept-Encoding").find("gzip") != std::string::npos);

    if (method != pion::http::types::REQUEST_METHOD_GET || !params.empty() || url != m_resource)
    {
        // bad request
        static const std::string badRequestData = std::string("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>") +
                "<document version=\"" + m_serviceName + "\">" +
                "<error type=\"bad request\"><description></description></error></document>";

        pion::http::response_ptr responsePtr(new pion::http::response(pion::http::types::REQUEST_METHOD_GET));
        responsePtr->set_status_code(pion::http::types::RESPONSE_CODE_BAD_REQUEST);
        responsePtr->set_status_message(pion::http::types::RESPONSE_MESSAGE_BAD_REQUEST);
        responsePtr->set_content_type("text/xml;charset=UTF-8");
        responsePtr->add_header(pion::http::types::HEADER_CONNECTION, "close");

        std::string response = badRequestData;
        if (isGzipEnabled)
        {
            try
            {
                response = Tools::CompressUtils::gzipStringCompress(badRequestData);
                responsePtr->add_header(pion::http::types::HEADER_CONTENT_ENCODING, "gzip");
            }
            catch (...)
            {
                response = badRequestData;
            }
        }

        responsePtr->set_content_length(response.length());
        responsePtr->set_content(response);

        contextPtr->sendResponse(responsePtr);
        return;
    }

    std::ostringstream xmlStream;

    xmlStream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << "\n"
            << "<document version=\"" << m_serviceName << "-" << m_serviceVersion << "\">" << "\n"
            << "<system>" << "\n"
            << "<version>" << m_serviceVersion << "</version>" << "\n"
            << "<revision>" << m_serviceRevision << "</revision>" << "\n"
            << "<start_time>" << boost::posix_time::to_iso_extended_string(m_startTime) << "</start_time>" << "\n"
            << "</system>" << "\n";

    xmlStream << "<user>" << "\n";
    BOOST_FOREACH(const ParameterIDs::value_type &value, m_parameterIDs)
    {
        xmlStream << "<" << value.first << ">" << m_parameterValues[value.second] << "</" << value.first << ">" << "\n";
    }

    boost::unique_lock<boost::mutex> lockProvider(m_mutexProvider);
    BOOST_FOREACH(ParametersProviders::value_type &provider, m_parametersProviders)
    {
    	Parameters parameters;
    	provider.second(parameters);

    	if (!parameters.empty())
    	{
    		xmlStream << "<" << provider.first << ">" << "\n";
    		BOOST_FOREACH(const Parameters::value_type &parameter, parameters)
    		{
    			xmlStream << "<" << parameter.first << ">" << parameter.second << "</" << parameter.first << ">" << "\n";
    		}
    		xmlStream << "</" << provider.first << ">" << "\n";
    	}
    }
    lockProvider.unlock();

    xmlStream << "</user>" << "\n"
            << "</document>";
    const std::string xmlResponse = xmlStream.str();

    pion::http::response_ptr responsePtr(new pion::http::response(pion::http::types::REQUEST_METHOD_GET));
    responsePtr->set_status_code(pion::http::types::RESPONSE_CODE_OK);
    responsePtr->set_status_message(pion::http::types::RESPONSE_MESSAGE_OK);
    responsePtr->set_content_type("text/xml;charset=UTF-8");

     if (isGzipEnabled)
     {
         std::string compressedXmlResponse;
         try
         {
             compressedXmlResponse = Tools::CompressUtils::gzipStringCompress(xmlResponse);
             responsePtr->add_header(pion::http::types::HEADER_CONTENT_ENCODING, "gzip");
         }
         catch(...)
         {
             compressedXmlResponse = xmlResponse;
         }
         responsePtr->set_content_length(compressedXmlResponse.length());
         responsePtr->set_content(compressedXmlResponse);
     }
     else
     {
         responsePtr->set_content_length(xmlResponse.length());
         responsePtr->set_content(xmlResponse);
     }

     contextPtr->sendResponse(responsePtr);
}

//--------------------------------------------------------------------------------------------------
void StatService::registerParameter(const std::string &name)
{
    if (isRunning())
    {
        throw std::runtime_error("Can't register parameter when service running.");
    }

    if (m_parameterIDs.find(name) != m_parameterIDs.end())
    {
        throw std::runtime_error("Parameter " + name + " already registered.");
    }

    const unsigned ID = m_parameterIDs.size();
    m_parameterIDs[name] = ID;
    m_parameterValues.resize(m_parameterIDs.size(), 0u);
}

//--------------------------------------------------------------------------------------------------
void StatService::set(const std::string &name, const long value)
{
    ParameterIDs::const_iterator it = m_parameterIDs.find(name);
    if (it == m_parameterIDs.end())
    {
        throw std::runtime_error("Parameter " + name + " not registered.");
    }

    BOOST_ASSERT(it->second < m_parameterValues.size());

    m_parameterValues[it->second] = value;
}

//--------------------------------------------------------------------------------------------------
void StatService::increment(const std::string &name, const long value)
{
    ParameterIDs::const_iterator it = m_parameterIDs.find(name);
    if (it == m_parameterIDs.end())
    {
        throw std::runtime_error("Parameter " + name + " not registered.");
    }

    BOOST_ASSERT(it->second < m_parameterValues.size());

    atomic_inc(&m_parameterValues[it->second], value);
}
//--------------------------------------------------------------------------------------------------
void StatService::registerParametersProvider(const std::string &name,
                                             const ParametersProvider &parametersProvider)
{
	if (name.empty())
	{
		throw std::runtime_error("Empty name of provider paramerers");
	}

	boost::unique_lock<boost::mutex> lockProvider(m_mutexProvider);
	ParametersProviders::const_iterator it = m_parametersProviders.find(name);
	if (it != m_parametersProviders.end())
	{
		throw std::runtime_error("Parameter " + name + " already registered.");
	}
	m_parametersProviders[name] = parametersProvider;
}
//--------------------------------------------------------------------------------------------------
void StatService::unregisterParametersProvider(const std::string &name)
{
	boost::unique_lock<boost::mutex> lockProvider(m_mutexProvider);
	ParametersProviders::iterator it = m_parametersProviders.find(name);
	if (it == m_parametersProviders.end())
	{
		throw std::runtime_error("Parameter " + name + " not registered.");
	}
	m_parametersProviders.erase(it);
}
//--------------------------------------------------------------------------------------------------
bool StatService::isRunning() const
{
    return m_isRunning;
}

} /* namespace WebServer */
} /* namespace Tools */
