#include <cstddef>

char *ft_strchr(const char *s, int c)
{
    if (!s)
        return NULL;

    char ch = static_cast<char>(c);

    while (*s) {
        if (*s == ch)
            return const_cast<char*>(s);
        ++s;
    }

    // check if we're searching for the null terminator
    if (ch == '\0')
        return const_cast<char*>(s);
	else
		return NULL;
 } 
