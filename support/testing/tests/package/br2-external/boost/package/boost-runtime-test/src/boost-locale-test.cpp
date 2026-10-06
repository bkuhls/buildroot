#include <boost/locale.hpp>

#include <iostream>
#include <string>

int main()
{
    // "grüße" in UTF-8
    const std::string utf8 = "gr\xc3\xbc\xc3\x9f" "e";

    std::wstring wide = boost::locale::conv::utf_to_utf<wchar_t>(utf8);
    if (wide.size() != 5)
        return 1;
    if (boost::locale::conv::utf_to_utf<char>(wide) != utf8)
        return 1;

    boost::locale::generator gen;
    std::cout.imbue(gen("en_US.UTF-8"));
    std::cout << boost::locale::format("locale: {1} characters") % wide.size() << std::endl;
    return 0;
}
