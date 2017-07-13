#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>

int main()
{
    using namespace std;

    ifstream file("oo.gz", ios_base::in | ios_base::binary);
    filtering_streambuf<input> in;
    in.push(zlib_decompressor());
    in.push(file);
    boost::iostreams::copy(in, cout);
    return 0;
}
