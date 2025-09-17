#include "webserv.hpp"
#include <string>
#include <cstddef>
#include <unistd.h>


std::string	connection_type(Client &client)
{
	std::string connection = client.get_header("Connection");
	if (!connection.empty())
		return connection;
	else if (client.get_request().http_version == "HTTP/1.1")
		return "keep-alive";
	else
		return "close";
}

//check whats the suffix after the dot and map content type/ "MIME" type to that
std::string get_content_type(const std::string &path)
{
    int dot_pos = path.rfind('.');
    if (dot_pos == (int)std::string::npos)
        return "application/octet-stream"; // generic MIME type for unknown binary data
    std::string suffix = path.substr(dot_pos + 1);
    if (suffix == "html" || suffix == "htm")
        return "text/html";
    else if (suffix == "css")
        return "text/css";
    else if (suffix == "js")
        return "application/javascript";
    else if (suffix == "jpg" || suffix == "jpeg")
        return "image/jpeg";
    else if (suffix == "png")
        return "image/png";
    else if (suffix == "gif")
        return "image/gif";
    else if (suffix == "txt")
        return "text/plain";
    else if (suffix == "pdf")
        return "application/pdf";
    // TODO mb add more types
    else
        return "application/octet-stream";
}

t_location *find_location(std::string uri, const t_server &config)
{
	printf("__find location__\n");
	printf("uri is: '%s'\n", uri.c_str());
	printf("this is location count '%d'\n", config.location_count);
	t_location	*best_match;
	int		best_len;
	int 		i;

	best_match = NULL;
	best_len = 0;
	i = 0;
	write(1, "1\n", 2);
	while (i < config.location_count)
	{
		write(1, "2\n", 2);
		t_location *l = config.locations[i];
		write(1, "3\n", 2);
		if (l && l->path)
		{
			write(1, "4\n", 2);
			std::string path(l->path);
			write(1, "5\n", 2);
			if (uri.rfind(path, 0) == 0)
			{
				write(1, "6\n", 2);
				if ((int)path.length() > best_len)
				{
					write(1, "7\n", 2);
					best_len = path.length();
					write(1, "8\n", 2);
					best_match = l;
					write(1, "9\n", 2);
				}
			}
		}
		i++;
	}
	if (best_match)
		printf("MATCH FOUND!\n");
	return (best_match);
}

bool	method_allowed(const std::string& method, const t_location *location)
{
	int i;

	i = 0;
	while (i < location->method_count)
	{
		if (location->accepted_methods[i] == method)
			return true;
		i++;
	}
	return false;
}


