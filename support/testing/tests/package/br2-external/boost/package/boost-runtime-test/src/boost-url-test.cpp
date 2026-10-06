#include <boost/url.hpp>

#include <iostream>

namespace urls = boost::urls;

int main()
{
    auto parsed = urls::parse_uri(
        "https://buildroot.org:8080/downloads/index.html?ver=2026.11&arch=arm#top");
    if (!parsed)
        return 1;

    urls::url url = *parsed;
    if (url.host() != "buildroot.org" || url.port_number() != 8080)
        return 1;

    url.params().set("arch", "aarch64");
    url.remove_fragment();

    if (urls::parse_uri("not a uri"))
        return 1;

    std::cout << "url: " << url.buffer() << std::endl;
    return 0;
}
