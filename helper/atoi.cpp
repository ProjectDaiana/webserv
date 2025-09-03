/*convert string to integer, ignore initial whitespaces & consider signs*/


static int	ft_isspace(char c)
{
	if (c == 32 || (c >= 9 && c <= 13))
		return (1);
	else
		return (0);
}

static int	ft_whichsign(char c)
{
	if (c != 45 && c != 43)
		return (0);
	if (c == 45)
		return (-1);
	else
		return (1);
}

int	ft_atoi(const char *nptr)
{
	int	i;
	int	sign;
	int	res;

	i = 0;
	sign = 1;
	res = 0;
	while (ft_isspace(nptr[i]))
		i++;
	if (ft_whichsign(nptr[i]) != 0)
	{
		sign *= ft_whichsign(nptr[i]);
		i++;
	}
	while (nptr[i] && (nptr[i] >= 48 && nptr[i] <= 57))
	{
		res = (nptr[i] - '0') + (res * 10);
		i++;
	}
	return (sign * res);
}
