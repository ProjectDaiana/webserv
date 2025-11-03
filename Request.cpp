
#include "Request.hpp"

Request::Request() {
	_parsed_request.method.clear();
	_parsed_request.uri.clear();
	_parsed_request.path.clear();
	_parsed_request.query.clear();
	_parsed_request.http_version.clear();
	_parsed_request.headers.clear();
	_parsed_request.body.clear();
	s_parse_error.code = 200;
	s_parse_error.msg.clear();
	_parse_error.clear();
}

Request::~Request() {};

bool Request::is_cgi() const {
    if (_parsed_request.uri.find("/cgi-bin/") == std::string::npos)
		return false;

	size_t len = _parsed_request.uri.length();
    if (len >= 4 && _parsed_request.uri.substr(len - 3) == ".py") {
        char prev_char = _parsed_request.uri[len - 4];
        // Check for valid filename char
        return (prev_char >= 'a' && prev_char <= 'z') || 
               (prev_char >= 'A' && prev_char <= 'Z') || 
               (prev_char >= '0' && prev_char <= '9') || 
               prev_char == '_';
    }
    return false;
}

void Request::reset_struct() {
    _parsed_request.method.clear();
    _parsed_request.uri.clear();
    _parsed_request.path.clear();
    _parsed_request.query.clear();
    _parsed_request.http_version.clear();
    _parsed_request.headers.clear();
    _parsed_request.body.clear();

    s_parse_error.code = 200;
    s_parse_error.msg.clear();
    _parse_error.clear();
}


// Parser
bool Request::parse(const std::string& raw_request) { //maybe uptade this to take a string and not a reference to raw_request
	std::cout << "DEBUG: parse() called with " << raw_request.length() << " characters" << std::endl;
	if (raw_request.empty()) {
        return false;
    }

	size_t headers_end = raw_request.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
		s_parse_error.code = 400;
        s_parse_error.msg = "No CRLF CRLF found - malformed request";
        return false;
    }
	
	std::string headers = raw_request.substr(0, headers_end);
    std::string body = raw_request.substr(headers_end + 4);

	if (!parse_start_line(headers) ||
        !parse_headers(headers) ||
        !parse_body(body)) {
        return false;
    }
    
	std::cout << "DEBUG - Parsed method: '" << _parsed_request.method << "'" << std::endl;
    std::cout << "DEBUG - Parsed URI: '" << _parsed_request.uri << "'" << std::endl;
    std::cout << "DEBUG - Headers count: " << _parsed_request.headers.size() << std::endl;
    return true;
}

// Parse Start Line
bool Request::parse_start_line(const std::string &headers) {
 	std::istringstream stream(headers);
    std::string line;

    if (!std::getline(stream, line)) {
        s_parse_error.msg = "Missing request line";
        return false;
    }
	if (!line.empty() && line[line.size() - 1] == '\r')
    	line.erase(line.size() - 1, 1);

	// Split and store it in struct  (check iss >> method)
    std::istringstream line_stream(line);
    if (!(line_stream >> _parsed_request.method
                      >> _parsed_request.uri 
                      >> _parsed_request.http_version)) {
		s_parse_error.code = 400;
        s_parse_error.msg = "Malformed start line: " + line;
        return false;
    }

	std::string extra;
    if (line_stream >> extra) {
        s_parse_error.code = 400;
        s_parse_error.msg = "Extra data in request line: " + extra;
        return false;
    }
	

    if (_parsed_request.method.empty() || 
        _parsed_request.uri.empty() || 
        _parsed_request.http_version.empty()) {
		s_parse_error.code = 400;
        s_parse_error.msg = "Incomplete start line";
        return false;
    }

	if (!is_method_uppercase(_parsed_request.method)) {
		s_parse_error.code = 400;
		s_parse_error.msg = "Method must be uppercase: " + _parsed_request.method;
		return false;
	}

	// if (!is_method_allowed(_parsed_request.method)) {
    //     s_parse_error.code = 405;
    //     s_parse_error.msg = "Method not allowed: " + _parsed_request.method;
    //     return false;
    // }

	if (!is_uri_valid(_parsed_request.uri) || !parse_uri(_parsed_request.uri)) {
		if (_parsed_request.uri.empty()) {
			s_parse_error.code = 400;
			s_parse_error.msg = "Empty URI";
		}
        return false;
	} //OJO if uri is invalid headers and body might still be parsed

    return true;
}


bool Request::is_method_uppercase(const std::string &method) const {
    for (size_t i = 0; i < method.length(); ++i) {
        if (method[i] < 'A' || method[i] > 'Z') {
            return false;
        }
    }
    return true;
}

// bool Request::is_method_allowed(const std::string &method) const {
// 	std::set<std::string> valid_methods;
// 	    if (valid_methods.empty()) {
//         valid_methods.insert("GET");
//         valid_methods.insert("POST");
//         valid_methods.insert("DELETE");
//         // TODO: Load valid methods from config file later
//     }
// 	return valid_methods.find(method) != valid_methods.end();
// }

bool Request::is_uri_valid(const std::string &uri) {
	if (uri[0] != '/') {
		s_parse_error.code = 400;
		s_parse_error.msg = "URI doesn't start with '/'";
		return false;
	}

	if (uri.length() > 8000) {
        s_parse_error.code = 414;
        s_parse_error.msg = "URI too long (> 8000 characters)";
        return false;
    }

	std::string::size_type first = uri.find('?');
    if (first != std::string::npos &&
        uri.find('?', first + 1) != std::string::npos) {
        return false;
    }
	return true;
}

bool Request::parse_uri(const std::string &uri) {
    std::string::size_type pos = uri.find('?');
    if (pos == std::string::npos) {
        _parsed_request.path = uri;
        _parsed_request.query = "";
    } else {
        _parsed_request.path = uri.substr(0, pos);
        _parsed_request.query = uri.substr(pos + 1);
    }
	normalize_path(_parsed_request.path);

    return true;
}

void Request::normalize_path(std::string& path) {
    // Collapse consecutive slashes: ///// → /
    size_t write_pos = 1;
    for (size_t read_pos = 1; read_pos < path.length(); ++read_pos) {
        if (path[read_pos] != '/' || path[write_pos - 1] != '/') {
            path[write_pos++] = path[read_pos];
        }
    }
    path.resize(write_pos);
}

// Parse Headers
bool Request::parse_headers(const std::string &headers) {
    std::istringstream stream(headers);
    std::string line;

    // Skip the start line (first line)
    if (!std::getline(stream, line))
        return false;

    while (std::getline(stream, line)) {
        // Remove trailing \r if present
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Empty line = end of headers
        if (line.empty())
            break;

        // Find the first colon
        std::string::size_type pos = line.find(':');
        if (pos == std::string::npos) {
			s_parse_error.code = 400;
            s_parse_error.msg = "Malformed header line: " + line;
            return false;
        }

        std::string name = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

		if (name.empty()) {
        	s_parse_error.code = 400;
        	s_parse_error.msg = "Empty header name";
        return false;
		}

        // Trim leading spaces in value
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        // Store in the map
        _parsed_request.headers[name] = value;
    }

    return true;
}


// Parse Body
bool Request::parse_body(const std::string &body_section) {
    // // Check for Transfer-Encoding first
    // std::map<std::string, std::string>::iterator te_it = _parsed_request.headers.find("Transfer-Encoding");
    // if (te_it != _parsed_request.headers.end()) {
    //     if (te_it->second.find("chunked") != std::string::npos) {
    //         // TODO: Parse chunked encoding
    //         _parsed_request.body = body_section; // Temporary
    //         return true;
    //     }
    // }
    
    // Check for Content-Length
    std::map<std::string, std::string>::iterator cl_it = _parsed_request.headers.find("Content-Length");
    if (cl_it != _parsed_request.headers.end()) {
        int length = atoi(cl_it->second.c_str());
        
        if ((int)body_section.size() < length) {
            // C++98 doesn't have std::to_string, so use stringstream
            std::ostringstream oss;
            oss << "Body truncated (expected " << length << " bytes, got " << body_section.size() << ")";
            s_parse_error.msg = oss.str();
            return false;
        }
        
        _parsed_request.body = body_section.substr(0, length);
    } else {
        // No Content-Length and no Transfer-Encoding = no body expected
        _parsed_request.body = "";
    }
    
    return true;
}

//Getters
const t_request& Request::get_parsed_request() const {
    return _parsed_request;
}


// Debug
void Request::print_struct() const {
	std::cout << "===== Here is the request struct =====" << std::endl;
	std::cout << _parsed_request.method << " "
			  << _parsed_request.uri << " "
			  << _parsed_request.http_version << "\n";
	std::cout << "Path: " << _parsed_request.path << "\n";
	std::cout << "Query: " << _parsed_request.query << "\n";
	std::cout << "Cgi?: " << is_cgi() << "\n";

	for (std::map<std::string, std::string>::const_iterator it = _parsed_request.headers.begin();
			it != _parsed_request.headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << "\n";
	}

	if (!_parsed_request.body.empty()) {
		std::cout << "\nBody:\n" << _parsed_request.body << "\n";
	}
	std::cout << "===== End of request struct ==========" << std::endl;
}

const s_error& Request::get_parse_error() const {
    return s_parse_error;
}
