#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <cstdlib>

class Client {
	private:
		int _fd;
		std::string _raw_request;
		bool _headers_complete;
		bool _read_complete;
		size_t _content_len;
		size_t _headers_end_pos;

	public:
		Client();
		Client(int fd);
		~Client();

		void add_to_request(char* data, int len);
		std::string get_request();
		bool get_read_complete() const;

		void print_raw_request() const;
};

#endif
