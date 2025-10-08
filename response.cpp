#include "webserv.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <stdio.h>
#include "Request.hpp"

std::string	handle_method(Client &client, const t_server &config, t_location *location)
{
	printf("\n__handling method__\n");
	printf("method is:  '%s'\n", client.get_request().method.c_str());
	if (client.get_request().method == "GET" && method_allowed("GET", location, client))
		return handle_get(client, config, location);
	else if (client.get_request().method == "POST" && method_allowed("POST", location, client))
		return handle_post(client, config, location);
	else if (client.get_request().method == "DELETE" && method_allowed("DELETE", location, client))
		return handle_delete(client, config, location);
	client.set_error_code(405);
	return std::string();
}

t_response	build_response(Client &client, const t_server &config)
{
	t_response res;
	t_location *location;
	//TODO how to handle methods(get) in case of redirect? default: just do nothing for now, see if it works magically

	printf("\n\n\n\n\n__TESTING RESPONSE__\n");
	printf("client fd: '%d'\n", client.get_fd());
	printf("is config accessible: server name is: '%s'\n", config.name);

	location = handle_location(client, config);
	res.location = check_redirect(location, client);
	res.version = client.get_request().http_version;
	if (client.is_cgi() && !client.cgi_output.empty())
		res.body = client.cgi_output;
	else if (!client.is_cgi())
		res.body = handle_method(client, config, location); 
	//TODO integrate this into redirect logic when everything is working
	if (client.get_method() == "GET")
		res.content_type = get_content_type(client); //TODO check content type for del & post
	else
	{ 
		client.set_error_code(303); //TODO test if content type still works
		res.location = reload_page(client);
	}
	printf("content type is: '%s'\n", res.content_type.c_str());
	printf("_______________________\nthis is uri: '%s'\n", client.get_path().c_str());
	res.connection = connection_type(client); //TODO change function name
	if (res.connection == "keep-alive")
	{
		printf("KEEP-ALIVE\n");
		client.set_keep_alive(true);
	}
	else
	{
		printf("KILLED CONNECTION\n");
		client.set_keep_alive(false);
	}
	res.content_length = res.body.size();
	res.status_code = client.get_error_code();
	res.reason_phrase = get_reason_phrase(res.status_code);
	//if (client.get_error_code() == 404) //TODO generic ft that checks if error code gets error page, redirect eg doesnt
		//TODO error pages logic, mb recursive?
	return (res);
}

void	handle_client_write(Client &client, const t_server &config)
{
	t_response response;
	std::stringstream sstr;
	int written = 0;

	response = build_response(client, config);
	sstr << response.version << " "
		<< response.status_code << " "
		<< response.reason_phrase << "\r\n";
	if (!response.location.empty())
		sstr << "Location: " << response.location << "\r\n";
	sstr << "Content-Type: " << response.content_type << "\r\n"
		<< "Content-Length: " << response.content_length << "\r\n"
		<< "Connection: " << response.connection << "\r\n"
		<< "\r\n" 
		<< response.body;
	std::string str_response(sstr.str());
		printf("\n_______________________________\n");
		printf("finished response:\n");
	//	printf("status code is: '%d'\n", response.status_code);
        write(1, str_response.c_str(), str_response.size());
		if (written == (int)str_response.size())
		{
			if (client.is_cgi())
				client.cgi_output.clear();
			printf("WRITE COMPLETE\n____________________________\n");
			client.set_write_complete(1);
		}
}
