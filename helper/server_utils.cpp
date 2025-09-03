#include <stdint.h>
#include <arpa/inet.h>
#include "webserv.hpp"

uint32_t iptoi(const char *ip_str)
{
    uint32_t result = 0;
    uint32_t octet = 0;
    int count = 0;
    const char *p = ip_str;

    while (*p && count < 4)
    {
        octet = ft_atoi(p);
        result = (result << 8) | (octet & 0xFF);
        count++;

        while (*p && *p != '.')
            p++;
        if (*p == '.')
            p++;
    }

    return htonl(result);
}

