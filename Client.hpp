#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include "Request.hpp"

class Client {
	private:
		int _fd;
		std::string _raw_request;
		bool _headers_complete;
		bool _read_complete;
		bool _is_parsed;
		size_t _content_len;
		size_t _headers_end_pos;

		Request _request;

	public:
		Client();
		Client(int fd);
		~Client();

		void add_to_request(char* data, int len);
		bool parse_request();
	
		// Getters
		bool is_read_complete() const;
		bool is_headers_complete() const;
		std::string& get_raw_request();
		size_t get_headers_end_pos() { return _headers_end_pos; };
		const std::string& get_method() const;
		const std::string& get_uri() const;
		const std::map<std::string, std::string>& get_headers() const;
		const std::string& get_body() const;
		const std::string& get_parse_error() const;

		// Debug
		void print_raw_request() const;
		void print_request_struct() const;
};

#endif
