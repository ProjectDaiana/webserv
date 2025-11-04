#include <stddef.h>


void *ft_memcpy(void *dest, const void *src, size_t n)
{
    if (!dest || !src)
        return NULL;

    unsigned char *d = static_cast<unsigned char*>(dest);
    const unsigned char *s = static_cast<const unsigned char*>(src);

    for (size_t i = 0; i < n; ++i)
        d[i] = s[i];

    return dest;
}
