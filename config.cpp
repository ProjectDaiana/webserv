#include "webserv.hpp"
#include <cerrno>      // errno
#include <cstddef>     // size_t
#include <cstdlib>     // exit(), EXIT_FAILURE
#include <cstring>     // strerror()
#include <fcntl.h>     // open() and O_RDONLY
#include <iostream>    // std::cout, std::cerr, std::string
#include <string>      // std::string (if not included by iostream)
#include <sys/stat.h>  // struct stat, stat()
#include <sys/types.h> // ssize_t
#include <unistd.h>    // read(), close()

std::string read_config(const char *path)
{
	int			fd;
	struct stat	st;
	ssize_t		bytes_read;

	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		exit(0); // OJO delete all the exits
		// ft_error("Error: Config file can't be opened!\n");
	}
	if (fstat(fd, &st) < 0)
	{
		close(fd);
		exit(0);
		// ft_error("Error: Cannot stat config file!\n");
	}
	std::string content;
	content.resize(st.st_size);
	bytes_read = read(fd, &content[0], st.st_size);
	if (bytes_read < 0)
	{
		close(fd);
		exit(0);
		// ft_error("Error: failed to read config file!\n");
	}
	close(fd);
	return (content);
}

void	autosettings(t_data *d)
{
	int i;
	int j;

	i = 0;
	if (!d->server_count)
		//ft_error("Error: no servers have been configured\n", 1);
		;
	while (i < d->server_count)
	{
		j = 0;
		if (!d->s[i]->lb)
			//ft_error("Error: server has no listen binding\n", 1);
			;
		if (!d->s[i]->max_bdy_size)
			 d->s[i]->max_bdy_size = 10 * 1024 * 1024;
		if (!d->s[i]->location_count)
			//ft_error("Error: server has no locations\n", 1);
			;
		while (j < d->s[i]->location_count)
		{
			if (!d->s[i]->locations[j]->path || !d->s[i]->locations[j]->root)
				//ft_error("Error: location without valid path", 1);
				;
			if (!d->s[i]->locations[j]->method_count)
				//ft_error("Error: location must allow at least one method", 1);
				;
			if (!d->s[i]->locations[j]->autoindex && !d->s[i]->locations[j]->default_file)
				//ft_error("Error: allow autoindexing or provide a default file", 1);
				;
			j++;
		}
		i++;
	}
}

void	init_config(t_data *d, t_arena *mem)
{
	t_lexer		*lx;
	t_parser	*ps;

	(void)d;
	lx = (t_lexer *)arena_alloc(mem, sizeof(t_lexer));
	ps = (t_parser *)arena_alloc(mem, sizeof(t_parser));
	std::string config_content = read_config("config/webserv.conf");
	lexer(lx, config_content, mem);
	parser(d, ps, lx, mem);
	autosettings(d); //TODO give all the rest of the server/location variables values if they dont have that
}
