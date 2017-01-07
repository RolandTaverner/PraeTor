// C && C++ && POSIX
#include <fcntl.h>
#include <locale>

// THIS
#include "Tools/Logger/LoggerFileStream.h"

// BOOST
#include <boost/regex.hpp>

namespace Conf = ::Tools::Configuration;

namespace Tools
{
namespace Logger
{

LoggerBaseStreamPtr LoggerFileStream::newStream(const Conf::ConfigurationView &fileConfig)
        
{
    try
    {
        boost::shared_ptr<LoggerBaseStream> pFile(new LoggerFileStream());
        pFile->configure(fileConfig);
        return pFile;
    }
    catch (const std::bad_alloc &)
    {
        throw LoggerError("Bad alloc memory for LoggerFileStream");
    }
    catch (const LoggerError &)
    {
        throw;
    }
    catch (...)
    {
        throw LoggerError("Error create LoggerFileStream");
    }
    return LoggerBaseStreamPtr();
}

//--------------------------------------------------------------------------------------------------
LoggerFileStream::LoggerFileStream()
{
    m_rotateMode = RM_NULL;
    m_periodSeconds = 0;
    m_nextSeconds = 0;
    m_size = 0;
    m_maxNumberFile = m_maxNumberFileDefault;

    m_pOutputFacet = new boost::posix_time::time_facet();
    m_pOutputFacet->format("%Y-%m-%dT%H:%M:%S");
    m_timeStampStream.imbue(std::locale(std::locale::classic(), m_pOutputFacet));
}

//--------------------------------------------------------------------------------------------------
LoggerFileStream::~LoggerFileStream() 
{
    if (m_file.is_open())
    {
        m_file.close();
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerFileStream::flush() 
{
    try
    {
        m_file << std::endl;
    }
    catch (...)
    {
        ;
    }
}

//--------------------------------------------------------------------------------------------------
bool LoggerFileStream::doRotation()
{
	namespace fs = boost::filesystem;

    m_file.clear();
    if (m_file.is_open())
    {
        m_file.close();
    }
    m_file.clear();

    const boost::posix_time::ptime now = boost::posix_time::microsec_clock().local_time();
    m_timeStampStream.clear();
    m_timeStampStream.str("");
    m_timeStampStream << now;

    if (!m_timeStampStream)
    {
        onError("Error write to time buffer for rotate file", m_errorMode);
        return false;
    }
    const std::string newFile = m_currentFile + "." + m_timeStampStream.str();
    if (rename(m_currentFile.c_str(), newFile.c_str()))
    {
        onError("Error rename file for rotate file", m_errorMode);
        return false;
    }
    setFileName(m_currentFile);

    m_files.push(fs::path(newFile));
    if (m_files.size() > m_maxNumberFile)
    {
    	if(fs::exists(m_files.front()))
    	{
    		fs::remove(m_files.front());
    	}
    	m_files.pop();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
void LoggerFileStream::rotate() 
{
    try
    {
        switch (m_rotateMode)
        {
            case RM_PERIOD:

                //struct timespec timeCurrent;

                //if (clock_gettime(CLOCK_MONOTONIC, &timeCurrent) != 0)
                //{
                //    onError("Error gettime for rotate logger", m_errorMode);
                //    return;
                //}

                //if (timeCurrent.tv_sec >= m_nextSeconds)
                //{
                //    m_nextSeconds += m_periodSeconds;

                //    if (!doRotation())
                //    {
                //        return;
                //    }
                //}
                break;

            case RM_SIZE:

                m_file.clear();
                if (m_size < m_file.tellp())
                {
                    if (!doRotation())
                    {
                        return;
                    }
                }
                break;

            default:
                break;
        };
    }
    catch (const LoggerError &e)
    {
        onError(e.what(), m_errorMode);
    }
    catch (...)
    {
        onError("Error rotate file stream", m_errorMode);
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerFileStream::setFileName(const std::string &fileName,
                                   std::ios_base::openmode mode) 
{
    try
    {
        if (fileName.empty())
        {
            onError("Empty path for file", m_errorMode);
            return;
        }

        m_file.clear();
        if (m_file.is_open())
        {
            m_file.close();
        }

        m_file.clear();
        m_file.open(fileName.c_str(), mode);

        if (m_file.is_open())
        {
            m_currentFile = fileName;
        }
        else
        {
            std::string error("Can't open log file: " + fileName);
            onError(error, m_errorMode);
        }
    }
    catch (LoggerError &e)
    {
        onError(e.what(), m_errorMode);
    }
    catch (...)
    {
        std::string error("Error setFileName for file stream" + fileName);
        onError(error, m_errorMode);
    }
}

//--------------------------------------------------------------------------------------------------
void LoggerFileStream::configure(const Conf::ConfigurationView &fileConfig) 
{
	namespace fs = boost::filesystem;

    try
    {
        m_errorMode = EM_EXCEPTION;

        const std::string nameFile = fileConfig.get("name");
        setFileName(nameFile);

        const boost::filesystem::path pathToFile = fs::path(nameFile);
        const boost::filesystem::path dirFile = pathToFile.parent_path();
		if (!fs::exists(dirFile) || !fs::is_directory(dirFile))
		{
			throw LoggerError("Not exist log directory '" + dirFile.string() + "'");
		}
		const std::string fileTemplate = pathToFile.filename().string()
				                        + "(\\.[\\d]{4}-[\\d]{1,2}-[\\d]{1,2}T[\\d]{1,2}:[\\d]{1,2}:[\\d]{1,2}){1}";

        m_pHeader.reset(ILoggerHeader::newLoggerHeader(fileConfig.get<std::string>("header", "short")));

        if (fileConfig.exists("rotate") && fileConfig.exists("rotate.period"))
        {
            //struct timespec timeCurrent;
            //m_periodSeconds =
            //        boost::posix_time::duration_from_string(fileConfig.get("rotate.period")).total_seconds();
            //if (clock_gettime(CLOCK_MONOTONIC, &timeCurrent) != 0)
            //{
            //    onError("Error gettime for rotate logger", m_errorMode);
            //}
            //m_nextSeconds = timeCurrent.tv_sec + m_periodSeconds;
            m_rotateMode = RM_PERIOD;

            if (fileConfig.exists("rotate.count"))
            {
            	m_maxNumberFile = fileConfig.get<long>("rotate.count");
            }
            creationBeginListFile(dirFile, fileTemplate, m_maxNumberFile);

            //std::cout << "\tPeriod rotate file: " << m_periodSeconds << " sec" << std::endl;
        };

        if (fileConfig.exists("rotate") && fileConfig.exists("rotate.size"))
        {
            m_size = fileConfig.get<long>("rotate.size") * 1000;
            m_rotateMode = RM_SIZE;

            if (fileConfig.exists("rotate.count"))
            {
            	m_maxNumberFile = fileConfig.get<long>("rotate.count");
            }
            creationBeginListFile(dirFile, fileTemplate, m_maxNumberFile);

            //std::cout << "\tSize rotate file: " << m_size << " byte" << std::endl;
        };

        const std::string errorString = fileConfig.get("error", std::string("stderr"));
        setErrorMode(errorString);
    }
    catch (LoggerError &e)
    {
        onError(e.what(), m_errorMode);
    }
    catch (...)
    {
        onError("Error configure for file stream", m_errorMode);
    }
}
//--------------------------------------------------------------------------------------------------
void LoggerFileStream::creationBeginListFile(const boost::filesystem::path &directory,
		                                     const std::string &fileTemplate,
		                                     const size_t maxNumberFileInList) 
{
	namespace fs = boost::filesystem;
	typedef std::pair<std::time_t, fs::path> FilePath;
	std::vector<FilePath> files;

	try
	{
		boost::regex rxFileName(fileTemplate);
		files.reserve(1000);

		if (fs::exists(directory) && fs::is_directory(directory))
		{
			fs::directory_iterator endIt;
			for( fs::directory_iterator dirIt(directory);
				 dirIt != endIt;
			     ++dirIt)
			{
				const fs::path &pathTofile = dirIt->path();
				if (fs::is_regular_file(pathTofile))
				{
					const std::string fileName = pathTofile.filename().string();
			        if (!boost::regex_match(fileName, rxFileName))
			        {
			            continue;
			        }

			        const std::time_t timeLastModification = fs::last_write_time(pathTofile);
			        files.push_back(std::make_pair(timeLastModification, pathTofile));
				}
			}
		}
		else
		{
			throw LoggerError("Not exist direcory '" + directory.string() + "'");
		}

		std::sort(files.begin(), files.end());

		std::list<fs::path> temporalList;
		std::vector<FilePath>::reverse_iterator it = files.rbegin();
		std::vector<FilePath>::reverse_iterator itEnd = files.rend();
		for (size_t count = 1; it != itEnd; ++it, ++count)
		{
			const fs::path &pathToFile = it->second;

			if (count <= maxNumberFileInList)
			{
				temporalList.push_back(pathToFile);
			}
			else if(fs::exists(pathToFile))
			{
				fs::remove(pathToFile);
			}
		}

		std::list<fs::path>::reverse_iterator itList = temporalList.rbegin();
		std::list<fs::path>::reverse_iterator itListEnd = temporalList.rend();
		for (;itList != itListEnd; ++itList)
		{
			m_files.push(*itList);
		}
	}
	catch(const std::exception &exc)
	{
		onError(exc.what(), m_errorMode);
	}
	catch(...)
	{
		onError("Error begin creation of list file", m_errorMode);
	}
}
//--------------------------------------------------------------------------------------------------
void LoggerFileStream::writeHeader(const LoggerMessageLevel &messageLevel) 
{
    if (m_pHeader)
    {
        m_pHeader->write(*this, messageLevel);
    }
}

//--------------------------------------------------------------------------------------------------
LoggerFileStream &LoggerFileStream::write(const std::string &out) 
{
    operator<<(out);
    return *this;
}

//--------------------------------------------------------------------------------------------------
LoggerFileStream &LoggerFileStream::setMessageLevel(const LoggerMessageLevel &) 
{
    return *this;
}

} /* namespace Logger */
} /* namespace Tools */
