//
// Created by svakhreev on 28.12.16.
//

#include <functional>
#include <boost/format.hpp>

namespace cameramanagercore::database::adapters
{

using std::string;
using std::function;

using boost::format;
using boost::str;

string operator | (function<format()>&& f1,
                   function<string()>&& f2) { return str(f1() % f2()); }

string operator | (format f1,
                   function<string()>&& f2) { return str(f1 % f2()); }

string operator | (function<string()>&& f1,
                   function<string()>&& f2) { return str(format("%1% %2%") % f1() % f2()); }

string operator | (string s,
                   function<string()>&& f2) { return str(format("%1% %2%") % s % f2()); }

string operator | (function<string()>&& f1,
                   string s) { return str(format("%1% %2%") % f1() % s); }

string operator | (string s1,
                   string s2) { return str(format("%1% %2%") % s1 % s2); }

}