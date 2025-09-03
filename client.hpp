#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>

class Client {
	private:
		int _fd;
		std::string _request;
		bool _request_complete;

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
