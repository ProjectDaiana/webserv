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
		int _error_code;
		t_request request; //NOTE better to store by value, later let getter return references

	public:
		Client();
		Client(int fd);
		~Client();

		void add_to_request(char* data, int len);
		std::string get_request();
		bool get_read_complete() const;

		void print_raw_request() const;
		int	get_error_code() const;
		void	set_error_code(int code);
		const t_request& get_request() const;
		void set_request(const t_request& new_request);
};

#endif
