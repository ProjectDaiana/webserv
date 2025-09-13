#include "webserv.hpp"
#include <string>

int	ft_delete(std::string path)
{
	return 0;
	//TODO figure out how were supposed to delete
	//return -1 when error
	
}


std::string	handle_delete(Client &client, const t_server &configm t_location *l)
{
	std::string path = std::string(l->root) + client.get_request().uri;
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
	if (ft_delete(path.c_str() == -1)
		client.set_error_code(500); //server error
	return std::string(); //empty body anyways
}
