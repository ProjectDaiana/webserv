#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _request_complete(false) {}

Client::Client() : _fd(-1), _request_complete(false) {}

Client::~Client() {};

void Client::add_to_request(char *data, int len) {
	_request.append(data, len); //TODO check the request syntax, is \r\n\r\n the end of request? do not append what comes after end of request
	if (_request.find("\r\n\r\n") != std::string::npos) {
		std::cout << "--- Request completed" << std::endl;
		_request_complete = true;
	}
	else
		std::cout << "Reading request" << std::endl;
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
