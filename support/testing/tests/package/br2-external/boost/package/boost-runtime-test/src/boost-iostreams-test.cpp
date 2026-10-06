#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <iostream>
#include <sstream>
#include <string>

namespace io = boost::iostreams;

template <typename Compressor, typename Decompressor>
static bool round_trip(const std::string &input)
{
    std::stringstream compressed;
    {
        io::filtering_ostream out;
        out.push(Compressor());
        out.push(compressed);
        out << input;
    }
    if (compressed.str().size() >= input.size())
        return false;

    std::stringstream output;
    {
        io::filtering_istream in;
        in.push(Decompressor());
        in.push(compressed);
        io::copy(in, output);
    }
    return output.str() == input;
}

int main()
{
    std::string input(1000, 'x');
    input += "boost";

    if (!round_trip<io::gzip_compressor, io::gzip_decompressor>(input))
        return 1;
    if (!round_trip<io::bzip2_compressor, io::bzip2_decompressor>(input))
        return 1;

    std::cout << "iostreams: gzip and bzip2 round trip OK" << std::endl;
    return 0;
}
