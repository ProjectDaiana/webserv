bool ft_isspace(char c)
{
    return (c == ' '  ||  // space
            c == '\t' ||  // horizontal tab
            c == '\n' ||  // newline
            c == '\v' ||  // vertical tab
            c == '\f' ||  // form feed
            c == '\r');   // carriage return
}
