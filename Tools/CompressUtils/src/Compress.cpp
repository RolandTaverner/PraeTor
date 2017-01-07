// BOOST
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>

// THIS
#include "Tools/CompressUtils/Compress.h"

namespace Tools
{
namespace CompressUtils
{

//------------------------------------------------------------------------------
std::string stringCompress(const std::string &input, const std::string &compressType)
{
	namespace io = boost::iostreams;
	std::string output;
	io::filtering_ostream out;

	if (compressType == "zlib")
	{
		out.push(io::zlib_compressor());
	}
	else if (compressType == "gzip")
	{
		out.push(io::gzip_compressor());
	}
	else
	{
		throw std::runtime_error("Unknown compress type");
	}

	out.push(io::back_inserter(output));
	io::copy(boost::make_iterator_range(input), out);
	return output;
}

//------------------------------------------------------------------------------
std::string stringUncompress(const std::string &inputCompressed, const std::string &compressType)
{
    if (inputCompressed.empty())
    {
    	throw std::runtime_error("Can't uncompress empty string");
    }

	namespace io = boost::iostreams;
	std::string output;
	io::array_source src(inputCompressed.data(), inputCompressed.length());

	if (compressType == "zlib")
	{
		io::copy(io::compose(io::zlib_decompressor(), src), io::back_inserter(output));
	}
	else if (compressType == "gzip")
	{
		io::copy(io::compose(io::gzip_decompressor(), src), io::back_inserter(output));
	}
	else
	{
		throw std::runtime_error("Unknown compress type");
	}

	return output;
}

//------------------------------------------------------------------------------
std::string zlibStringCompress(const std::string &input)
{
	return stringCompress(input, "zlib");
}

//------------------------------------------------------------------------------
std::string zlibStringUncompress(const std::string &inputCompressed)
{
	return stringUncompress(inputCompressed, "zlib");
}

//------------------------------------------------------------------------------
std::string gzipStringCompress(const std::string &input)
{
	return stringCompress(input, "gzip");
}

//------------------------------------------------------------------------------
std::string gzipStringUncompress(const std::string &inputCompressed)
{
	return stringUncompress(inputCompressed, "gzip");
}

} // namespace CompressUtils
} // namespace Tools
