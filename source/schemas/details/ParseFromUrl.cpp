//
// Created by svakhreev on 30.03.17.
//

#include "ParseFromUrl.hpp"


namespace cameramanagercore::schemas
{

void url_decode(char *dst, const char *src)
{
    char a, b;
    while (*src)
    {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b)))
        {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16 * a + b;
            src+=3;
        }
        else if (*src == '+')
        {
            *dst++ = ' ';
            src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

std::unordered_map<std::string, std::string> ParseFromUrl(std::string str)
{
    using namespace ranges;
    std::unique_ptr<char[]> raw = std::make_unique<char[]>(str.length() + 1);
    url_decode(raw.get(), str.data());
    std::string encoded = raw.get();

    return encoded | view::split('&') >>= [] (std::string s)
        {
            std::vector<std::string> v = s | view::split('=');
            return ranges::yield_if(v.size() == 2, std::make_pair(v.front(), v.back()));
        };
}

}
