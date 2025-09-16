#include "webserv.hpp"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

//TODO include more headers

std::string autoindex_directory(Client &client, const std::string path)
{
	(void)client;
	(void)path;
	return std::string();
	//open the directory q opendir()
	//loop through entries w readdir()
	//generate a html line in an html string for each entry
	//close the directory w closedir()
	//return the html str as the body
}

std::string	file_to_str(Client &client, const std::string &path)
{
	std::string content_bytes; //can be chars in case its a text file or compressed image data in case its a picture
	int fd = open(path.c_str(), O_RDONLY);
	if (-1 == fd)
	{
		client.set_error_code(404);
		return content_bytes;
	}
	char buffer[4096]; //4kb
	ssize_t bytes_read; //ssize_t is signed size_t, which is useful for -1 error
	while ((bytes_read = read(fd, buffer, 4096)) > 0) //read bytes from file
		content_bytes.append(buffer, bytes_read); //keep appending them
	if (bytes_read == -1) //handle read error
	{
		client.set_error_code(500);
		content_bytes.clear();
	}
	close(fd);
	return content_bytes; //return buffer
}

std::string handle_get(Client &client, const t_server &config, t_location *l)
{
	(void)config; //TODO remove from ft if not needed
	struct stat st; //struct that stat fills with information about a file path
	std::string path = std::string(l->root) + client.get_path();
	if (stat(path.c_str(), &st) == -1) //if stat returns -1, the file doesnt exist, path not found
	{
		client.set_error_code(404);
		return std::string();
	}
	if (S_ISREG(st.st_mode)) //S_ISREG is the flag for a regular file, not a direcory
		return file_to_str(client, path);
	else if (S_ISDIR(st.st_mode)) //handle directories
	{
		if (l->autoindex) //if autoindexing is turned on, list files in directory
			return autoindex_directory(client, path); //TODO write this
		else if (l->default_file) //if its not on and theres a default file, serve it
		{
			std::string index_path = std::string(l->root) + "/" + std::string(l->default_file);
			if (stat(index_path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
				return file_to_str(client, index_path);
		}
	}
	client.set_error_code(403); //autoindex off && no default fine || path is neither file nor dir
	return std::string();
}
