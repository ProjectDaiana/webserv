#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
//#include "Response.hpp"

class Client {
	private:
		int _fd;
		std::string _raw_request;
		// s_request parsed_request;
		bool _headers_complete;
//		Response _response;

	public:
		Client();
		Client(int fd);
		~Client();

		void add_to_request(char* data, int len);
		std::string get_request();
		bool get_is_completed() const;

		void print_raw_request() const;
};

#endif
