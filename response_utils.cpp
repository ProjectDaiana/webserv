#include "webserv.hpp"
#include <string>
#include <cstddef>


std::string	get_connection_type(Client &client)
{
	const std::map<std::string, std::string>& headers = client.get_request().headers;
	std::string connection;

	std::map<std::string, std::string>::const_iterator i = headers.find("Connection");
	if (i != headers.end()) //in case the iterator hasnt reached the end, it has found "Connection"
	{
		connection = i->second; //second stands for the value associated with the key at i position
		return connection;
	}
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
	t_location	*best_match;
	int		best_len;
	int 		i;

	best_match = NULL;
	best_len = 0;
	i = 0;
	while (i < config.location_count)
	{
		t_location *l = config.locations[i];
		if (l && l->path)
		{
			std::string path(l->path);
			if (uri.rfind(path, 0) == 0)
			{
				if ((int)path.length() > best_len)
				{
					best_len = path.length();
					best_match = l;
				}
			}
		}
		i++;
	}
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


