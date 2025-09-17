#include "webserv.hpp"
#include "Client.hpp"
#include <unistd.h>


std::string	handle_method(Client &client, const t_server &config)
{
	t_location *location;
	
	location = find_location(client.get_request().uri, config); //TODO is this working correctly?
	if (!location)
	{
		client.set_error_code(404);
		return std::string();
	}
	else if (client.get_request().method == "GET" && method_allowed("GET", location))
		return handle_get(client, config, location);
	else if (client.get_request().method == "POST" && method_allowed("POST", location))
		return handle_post(client, config, location);
	else if (client.get_request().method == "DELETE" && method_allowed("DELETE", location))
		return handle_delete(client, config, location);
	client.set_error_code(405);
	return std::string();
}

t_response	build_response(Client &client, const t_server &config)
{
	t_response res;

	res.version = client.get_request().http_version;
	res.body = handle_method(client, config);
	res.content_type = get_content_type(client.get_request().uri);
	res.connection = get_connection_type(client); //TODO figure out correct key
	res.content_length = res.body.size();
	res.status_code = client.get_error_code();
	res.reason_phrase = get_reason_phrase(res.status_code);
	//TODO handle res.location for redirection
	//TODO integrate the parse errors, ideally in parser
	return (res);
}

void	handle_client_write(Client &client, const t_server &config)
{
	t_response response;
	std::stringstream sstr;
	
	response = build_response(client, config);
	sstr << response.version << " "
		<< response.status_code << " "
		<< response.reason_phrase << "\r\n"
		<< "Content-Type :" << response.content_type << "\r\n"
		<< "Content-Length " << response.content_length << "\r\n"
		<< "Connection: " << response.connection << "\r\n"
		<< "\r\n" 
		<< response.body;
	std::string str_response(sstr.str());
        write(client.get_fd(), str_response.c_str(), str_response.size()); //TODO we need to write in chunks later
}

//TODO look into chunking
