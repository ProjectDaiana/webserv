#include "Client.hpp"

Client::Client(int fd, Server &server) :
							_fd(fd), _headers_complete(0),\
							_read_complete(0), _write_complete(0),
							_is_parsed(false),
							_keep_alive(0), _content_len(0),
							_headers_end_pos(0),
							_error_code(200),
							_server(&server)
							{}

Client::Client() : _fd(-1), _is_parsed(false), _server(NULL) {};

Client::~Client() {};

void Client::add_to_request(char *data, int len) {
	_raw_request.append(data, len); 

	if (!_headers_complete ) {
		size_t pos = _raw_request.find("\r\n\r\n");
		if (pos != std::string::npos) {
			// std::cout << "DEBUG: Headers found at position " << pos << std::endl;
			_headers_complete = true;
			_headers_end_pos = pos + 4; 
			
			size_t cl = _raw_request.find("Content-Length:");
			if (cl != std::string::npos) {
				cl += 15; // skip string "Content-Length:"
				while (cl < _raw_request.size() && isspace(_raw_request[cl]))
					cl++;
				_content_len = std::atoi(_raw_request.c_str() + cl);
				// std::cout << "DEBUG: Content-Length found: " << _content_len << std::endl;
			}
			else {
				_content_len = 0;
				// std::cout << "DEBUG: No Content-Length found, setting to 0" << std::endl;
			}

			size_t body_size = _raw_request.size() - _headers_end_pos;
			// std::cout << "DEBUG: Total request size: " << _raw_request.size() << std::endl;
			// std::cout << "DEBUG: Headers end at: " << _headers_end_pos << std::endl;
			// std::cout << "DEBUG: Body size so far: " << body_size << std::endl;
			// std::cout << "DEBUG: Expected content length: " << _content_len << std::endl;

			if (body_size >= _content_len) {
				_read_complete = true;
				// std::cout << "DEBUG: READ_COMPLETE set to true immediately" << std::endl;
				// std::cout << "DEBUG: Content-Length found: " << _content_len << std::endl;
			}
		}	
		std::cout << "Reading request" << std::endl;
	}
	else {
		size_t body_size = _raw_request.size() - _headers_end_pos;
		// std::cout << "DEBUG: Adding more body data. Current body size: " << body_size 
			//   << ", expected: " << _content_len << std::endl;
		if (body_size >= _content_len) {
		_read_complete = true;
			// std::cout << "DEBUG: READ_COMPLETE set to true after additional data" << std::endl;
		}
	}
};

bool Client::parse_request() {
	// std::cout << "DEBUG: Client::parse_request called" << std::endl;
    // std::cout << "DEBUG: _is_parsed = " << _is_parsed << std::endl;
    // std::cout << "DEBUG: _headers_complete = " << _headers_complete << std::endl;
    // std::cout << "DEBUG: _read_complete = " << _read_complete << std::endl;
    // std::cout << "DEBUG: Raw request length = " << _raw_request.length() << std::endl;

	if (!_is_parsed && _read_complete) {
		_is_parsed = _request.parse(_raw_request);
	}
	set_error_code(_request.get_parse_error().code);
	return _is_parsed;
}

void Client::reset() {
	printf("Client '%d' is being reset\n", _fd);
	_raw_request.clear();
	_request.reset_struct();
	_headers_complete = false;
	_read_complete = false;
	_write_complete = false;
	_is_parsed = false;
	_keep_alive = false;
	_content_len = 0;
	_headers_end_pos = 0;
	_error_code = 200;
}

void Client::update_activity() {
	_last_activity = std::time(NULL);
}

time_t Client::get_last_activity() const {
	return _last_activity;
}

// Getters
std::string& Client::get_raw_request() {
	return _raw_request;
};

bool Client::is_read_complete() const {
	return (_read_complete);
}

bool Client::is_headers_complete() const {
	return (_headers_complete);
}

bool Client::is_cgi() const {
	return _request.is_cgi();
}

const std::string& Client::get_method() const {
	return _request.get_parsed_request().method;
}

const std::string& Client::get_uri() const {
	return _request.get_parsed_request().uri;
}

const std::string& Client::get_http_version() const {
	return _request.get_parsed_request().http_version;
}

const std::string& Client::get_path() const {
	return _request.get_parsed_request().path;
}

const std::string& Client::get_query() const {
	return _request.get_parsed_request().query;
}


const std::map<std::string, std::string>& Client::get_headers() const {
	return _request.get_parsed_request().headers;
}

const std::string& Client::get_header(const std::string& key) const {
    static const std::string empty = ""; // safe return if not found
    const std::map<std::string, std::string>& headers = get_headers();
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return empty;
}

const std::string& Client::get_body() const {
	return _request.get_parsed_request().body;
}

const s_error& Client::get_parse_error() const {
	return _request.get_parse_error();
}

int Client::get_fd() const
{
	return _fd;
}

// Debug
void Client::print_raw_request() const 
{
	std::cout << "\n=== Raw HTTP Request from client " << _fd << " ===\n";
	std::cout << _raw_request;
	// std::cout << "Length: " << _raw_request.length() << " chars\n";
	// std::cout << "Length: " << _raw_request.length() << " chars\n";
	std::cout << "=== End Raw Request ===\n\n";

}

void Client::print_request_struct() const {
	std::cout << "DEBUG: _read_complete = " << _read_complete << std::endl;
    _request.print_struct();
	std::time_t result =  get_last_activity() ;
	std::cout << "Timestamp: " << std::asctime(std::localtime(&result)) << std::endl;
}

int Client::get_error_code() const 
{
    return _error_code;
}

void Client::set_error_code(int code) 
{
	printf("ERROR CODE '%d' HAS BEEN SET!\n", code);
    if (_error_code == 200)
        _error_code = code;
}

const t_request& Client::get_request() const 
{
	return _request.get_parsed_request();
}

