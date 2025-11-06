#include "webserv.hpp"
#include <string>
#include <cstddef>
#include <unistd.h>
#include <cstdio>

std::string reload_page(t_location *location, Client &client)
{
	std::string referer = client.get_header("Referer");
	if (referer.empty())
		return std::string(location->default_file);
	else
		return referer;
}

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

#include <sys/stat.h>
#include <string>

bool is_directory(const std::string &path, t_location *location)
{
    struct stat info;
	std::string full_path;	

	full_path = std::string(location->root) + path;
	printf("directory is being tested w path: '%s'\n", path.c_str());
    if (stat(full_path.c_str(), &info) != 0)
	{
		printf("PATH NOT ACCESIBLE!\n");
        return false; // could not access path
	}
    return (info.st_mode & S_IFDIR) != 0;
}

//check whats the suffix after the dot and map content type/ "MIME" type to that
std::string get_content_type(Client &client, t_location *location) //TODO pass location to be able to test if its dir
{
    std::string path = client.get_path();
	if (is_directory(path, location))
	{
		printf(">>IS A DIRECTORY\n");
		return "text/html";
		exit(0);
	}
	else
		printf(">>NOT A DIRECTORY\n");
    if (client.is_cgi())
	    return "text/html";
    printf("GETTING CONTENT TYPE NOW W PATH: '%s'\n", path.c_str());	    
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
	else if (suffix == "mp3")
		return "audio/mpeg";
    else if (suffix == "pdf")
        return "application/pdf";
	else if (suffix == "ico")
		return "image/x-icon";
    // TODO mb add more types
	//TODO add video
    else
        return "application/octet-stream";
}

t_location *find_location(std::string uri, const t_server &config)
{
	// printf("__find location__\n");
	// printf("uri is: '%s'\n", uri.c_str());
	// printf("this is location count '%d'\n", config.location_count);
	t_location	*best_match;
	int		best_len;
	int 		i;

	best_match = NULL;
	best_len = 0;
	i = 0;
	//write(1, "1\n", 2);
	while (i < config.location_count)
	{
		//write(1, "2\n", 2);
		t_location *l = config.locations[i];
		//write(1, "3\n", 2);
		if (l && l->path)
		{
			//write(1, "4\n", 2);
			std::string path(l->path);
			//write(1, "5\n", 2);
			if (uri.rfind(path, 0) == 0)
			{
				//write(1, "6\n", 2);
				if ((int)path.length() > best_len)
				{
					//write(1, "7\n", 2);
					best_len = path.length();
					//write(1, "8\n", 2);
					best_match = l;
					//write(1, "9\n", 2);
				}
			}
		}
		i++;
	}
	if (best_match)
		printf("MATCH FOUND!\n");
	return (best_match);
}

std::string check_redirect(t_location *location, Client &client)
{
	if (!location->redirect)
		return std::string();
	else
	{
		client.set_error_code(301);
		return std::string(location->redirect);
	}
}

t_location *handle_location(Client &client, const t_server &config)
{
	if (client.get_path().empty())
		printf("ERROR: empty uri.\n"); //DEBUG
	t_location *location;

        location = find_location(client.get_request().uri, config);
	// if (location)
    //             printf("location passed!\n");
        if (!location)
        {
                client.set_error_code(404);
                printf("404 set at first instance");
                return NULL;
        }
	return location;
}

bool	method_allowed(const std::string& method, const t_location *location, Client &client)
{
	int i;

	i = 0;
	if (client.get_path() == "index.html" && (method == "GET" || method == "DELETE"))
		return false;
	while (i < location->method_count)
	{
		if (location->accepted_methods[i] == method)
			return true;
		i++;
	}
	printf("%s is not allowed!\n", method.c_str());
	return false;
}


