#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _headers_complete(0), _read_complete(0), _content_len(0), _headers_end_pos(0) {}

Client::Client() : _fd(-1) {}

Client::~Client() {};

void Client::add_to_request(char *data, int len) {
	_raw_request.append(data, len); 

	if (!_headers_complete ) {
		size_t pos = _raw_request.find("\r\n\r\n");
		if (pos != std::string::npos) {
			std::cout << "--- Headers completed" << std::endl;
			_headers_complete = true;
			_headers_end_pos = pos + 4; 
			
			size_t cl = _raw_request.find("Content-Length:");
			if (cl != std::string::npos) {
				cl += 15; // skip "Content-Length:"
				while (cl < _raw_request.size() && isspace(_raw_request[cl]))
					cl++;
				_content_len = std::atoi(_raw_request.c_str() + cl);
			}
		}	
		std::cout << "Reading request" << std::endl;
	}
	else {
		size_t body_size = _raw_request.size() - (_headers_end_pos);
		if (body_size >= _content_len)
			_read_complete = true;
	}
};

std::string Client::get_request() {
	return _raw_request;
};

bool Client::get_read_complete() const {
	return (_headers_complete);
}

void Client::print_raw_request() const {
	{
		std::cout << "\n=== Raw HTTP Request from client " << _fd << " ===\n";
		std::cout << _raw_request << std::endl;
  		std::cout << "Length: " << _raw_request.length() << " chars\n";
		std::cout << "=== End Request ===\n\n";
	}
}
