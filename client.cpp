#include "client.hpp"

Client::Client(int fd) : _fd(fd), _request_complete(false) {}

Client::~Client() {};

void Client::add_to_request(char *data, int len) {
	_request.append(data, len);
	_request_complete = true; /////// change this
};

std::string Client::get_request() {
	return _request;
};

bool Client::get_is_completed() const {
	return _request_complete;
}

void Client::print_raw_request() const {
	{
		std::cout << "\n=== Raw HTTP Request from client " << _fd << " ===\n";
		std::cout << _request << std::endl;
		std::cout << "=== End Request ===\n\n";
	}
}