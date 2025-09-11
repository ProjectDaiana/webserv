#include "webserv.hpp"
#include <cstring>
#include <iostream>
#include <sstream>  

#ifndef REQUEST_HPP
#define REQUEST_HPP

class Request {
	private:
		s_request _parsed_request;
    	std::string _parse_error;
		bool parse_start_line(const std::string &headers);
		bool parse_headers(const std::string &headers);
		bool parse_body(const std::string &body);

	public:
		Request();
		Request(const std::string &raw);
		~Request();
	
		bool parse(const std::string& raw_request);
		bool is_cgi();

		// Getters
		const s_request& get_parsed_request() const;
		const std::string& get_parse_error() const;

		// Debug
		void print_struct() const;
		
		
		// bool is_allowed_method(std::string method);
		// bool is_supported_http();
    	// bool has_header(const std::string& key) const;
	    // bool has_body() const;
    	// size_t get_content_length() const;
};

#endif