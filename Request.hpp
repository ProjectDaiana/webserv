#include "webserv.hpp"
#include <cstring>
#include <iostream>
#include <sstream>  
#include <set>
#include "Client.hpp"

#ifndef REQUEST_HPP
#define REQUEST_HPP
class Client;

struct s_error {
    int code;
    std::string msg;

	s_error() : code(0) {}
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
		void normalize_path(std::string& path);
	
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