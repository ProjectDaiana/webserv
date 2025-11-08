#include "webserv.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <stdio.h>
#include "Request.hpp"
#include "errors.hpp"

std::string	handle_method(Client &client, const t_server &config, t_location *location)
{
	if (client.get_request().method.empty())
		return std::string();
	if (client.get_request().method == "GET" && method_allowed("GET", location, client))
		return handle_get(client, config, location);
	else if (client.get_request().method == "POST" && method_allowed("POST", location, client))
		return handle_post(client, config, location);
	else if (client.get_request().method == "DELETE" && method_allowed("DELETE", location, client))
		return handle_delete(client, location, std::string());
	client.set_error_code(405);
	return std::string();
}

t_response	build_response(Client &client, const t_server &config)
{
	t_response res;
	t_location *location;

	location = handle_location(client, config);
	if(!client.is_cgi())
		res.location = check_redirect(location, client);
	res.version = client.get_request().http_version;
	if (res.version.empty())
		res.version = "HTTP/1.1";
	if (client.is_cgi() && !client.cgi_output.empty())
		res.body = client.cgi_output;
	else if (!client.is_cgi())
		res.body = handle_method(client, config, location); 
	res.content_type = get_content_type(client, location);

	if(!client.is_cgi() && client.get_method() != "GET")
	{ 
		client.set_error_code(303); 
		res.location = reload_page(location, client);
	}
	res.connection = connection_type(client); 
	if (res.connection == "keep-alive")
		client.set_keep_alive(true);
	else
		client.set_keep_alive(false);
	res.content_length = res.body.size();
	res.status_code = client.get_error_code();
	res.reason_phrase = get_reason_phrase(res.status_code);
	if (displays_error(client))
		load_error_page(client, &res, config, location);
	res.content_length = res.body.size();
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
	if (!(client.get_header("Host").empty()))
		sstr << "Host: " << client.get_header("Host") << "\r\n";
	if (!response.location.empty())
		sstr << "Location: " << response.location << "\r\n";
	sstr << "Content-Type: " << response.content_type << "\r\n"
		<< "Content-Length: " << response.content_length << "\r\n"
		<< "Connection: " << response.connection << "\r\n"
		<< "\r\n" 
		<< response.body;
	std::string str_response(sstr.str());
		written = write(client.get_fd(), str_response.c_str(), str_response.size());
       		//write(1, str_response.c_str(), str_response.size());
		if (written == (int)str_response.size())
		{
			if (client.is_cgi())
				client.cgi_output.clear();
			client.set_write_complete(1);
		}
}
