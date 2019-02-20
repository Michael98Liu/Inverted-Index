#include <string>
#include <strless.hpp>
using namespace std;

bool strless::operator() (const string & first, const string & second ) const  {
    return first < second;
}
