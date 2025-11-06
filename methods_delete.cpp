#include "webserv.hpp"
#include <string>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>


int	ft_delete(std::string path)
{
	if (std::remove(path.c_str()) != 0)
		return -1;
	return 0;
}


std::string	handle_delete(Client &client, t_location *l, std::string path)
{
	printf("DELETE WAS CALLED\n");
	if (path.empty() && l)
		path = std::string(l->root) + client.get_request().uri;
	if (path.find("..") != std::string::npos) //check that file is in root for security
   	{
       		client.set_error_code(403); // forbidden
        	return std::string();
    	}
	struct stat st;
	if (stat(path.c_str(), &st) < 0)
	{
		client.set_error_code(404); //not found
		return std::string();
	}
	if (!S_ISREG(st.st_mode)) //only allow one file to be deleted at a time, not a whole dir
	{
		client.set_error_code(403); //not allowed
		return std::string();
	}
	if (ft_delete(path) == -1)
		client.set_error_code(500); //server error
	return std::string(); //empty body anyways
}
