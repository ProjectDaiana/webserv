#include "webserv.hpp"
#include <fstream>

//list of error codes
std::string get_reason_phrase(int code) 
{
    switch (code) 
    {
        // Success
        case 200: return "OK";
        case 201: return "Created";
		case 204: return "No Content";

        // Redirection
        case 300: return "Multiple Choices";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";

        // Client errors
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 406: return "Not Acceptable";
        case 408: return "Request Timeout";
        case 411: return "Length Required";
        case 413: return "Payload Too Large";
        case 414: return "URI Too Long";
        case 415: return "Unsupported Media Type";

        // Server errors
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        case 504: return "Gateway Timeout";

        default:  return "Unknown";
    }
}



bool displays_error(Client &client)
{
	int code = client.get_error_code();
	return (code >= 400 && code < 600);
}

void load_error_page(Client &client, t_response *res, const t_server &config, t_location *location)
{
	printf("load error page has been called!\n");
	int code = client.get_error_code();
	int i = 0;
	std::string path;

	while (i < config.error_code_count)
	{
		if (config.error_codes[i] == code)
		{
			path = config.error_pages[i];
			printf("error page has been found!\n");
			break;
		}
		i++;
	}
	if (path.empty()) //if no error code available
	{
		std::stringstream body;
		body << "<html><body><h1>" << code << " "
             << get_reason_phrase(code) << "</h1></body></html>";			
		res->body = body.str();
		res->content_type = "text/html";
		return ;
	}
	// If no location (request not parsed), use first location's root as fallback
	std::string root_path;
	if (location)
		root_path = location->root;
	else if (config.location_count > 0)
		root_path = config.locations[0]->root;
	else
		root_path = "www/html";
	std::string full_path = root_path + path;
	printf("this is the path to error page: '%s'\n", full_path.c_str());
    std::ifstream file(full_path.c_str());
    if (!file.is_open())
    {
		printf("error page not found!");
        std::stringstream body;
        body << "<html><body><h1>" << code << " "
             << get_reason_phrase(code) << "</h1></body></html>";
        res->body = body.str();
        res->content_type = "text/html";
        return;
    }
    std::stringstream buf;
    std::string line;
    while (std::getline(file, line))
        buf << line << "\n";
    file.close();
    res->body = buf.str();
    res->content_type = "text/html";
}	

