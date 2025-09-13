#include "webserv.hpp"
#include <cstring>
#include <iostream>
#include <sstream>  
#include <set>

#ifndef REQUEST_HPP
#define REQUEST_HPP

typedef struct s_request t_request;

//NOTE cut off uri before "?" in parser, ignore query str
struct  s_request
{
    std::string method; //can only be one ofc, for example GET
    std::string uri; //will be eg: "/cgi-bin/test.py", since we dont handle query str (at least for now)
    std::string path;
    std::string query;
    std::string http_version; // eg: "HTTP/1.1", which version did the client use, impacts how we respond
    //TODO for which headers to implement -> check what each do and what we think makes sense to implement and what to leave out, also check subject if any headers are specifically required
    std::map<std::string, std::string> headers; //we should use a map here bc its easy to implement and use
    std::string body; //data the user is posting/putting into the website, for method post, so the body can be empty, depending on the request type
};

struct s_error {
    int code;
    std::string msg;

	s_error() : code(400), msg("Bad Request") {}
};

class Request {
	private:
		t_request _parsed_request;
    	s_error s_parse_error;
		std::map<std::string, std::string> _parse_error;
		bool parse_start_line(const std::string &headers);
		bool parse_uri(const std::string &uri);
		bool parse_headers(const std::string &headers);
		bool parse_body(const std::string &body);

		bool is_method_uppercase(const std::string &method) const;
		bool is_method_allowed(const std::string &method) const;
		bool is_uri_valid(const std::string &uri);
	
	public:
		Request();
		Request(const std::string &raw);
		~Request();
	
		bool parse(const std::string& raw_request);
		bool is_cgi();

		// Getters
		const t_request& get_parsed_request() const;
		const s_error& get_parse_error() const;

		// Debug
		void print_struct() const;
		
		// bool is_supported_http();
    	// bool has_header(const std::string& key) const;
	    // bool has_body() const;
    	// size_t get_content_length() const;
};

#endif
