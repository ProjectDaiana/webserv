
#include "Request.hpp"

Request::Request() {
	std::cout << "DEBUG: Request constructor called" << std::endl;
};

Request::~Request() {};

bool is_cgi() { //TODO implement after parser
	return std::rand() % 2 == 0;
	// return false;
}

// bool Request::is_cgi() const {
//     return (method == "GET" || method == "POST") 
//         && uri.find("/cgi-bin/") == 0;
// }

// Parser
bool Request::parse(const std::string& raw_request) {
	std::cout << "DEBUG: parse() called with " << raw_request.length() << " characters" << std::endl;
	if (raw_request.empty()) {
        return false;
    }

	size_t headers_end = raw_request.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
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
        s_parse_error.msg = "Malformed start line: " + line;
        return false;
    }
	// Check for emtpy values 
    if (_parsed_request.method.empty() || 
        _parsed_request.uri.empty() || 
        _parsed_request.http_version.empty()) {
        s_parse_error.msg = "Incomplete start line";
        return false;
    }

    return true;
}

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
            s_parse_error.msg = "Malformed header line: " + line;
            return false;
        }

        std::string name = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        // Trim leading spaces in value
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
            value.erase(0, 1);

        // Store in the map
        _parsed_request.headers[name] = value;
    }

    return true;
}

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
const s_request& Request::get_parsed_request() const {
    return _parsed_request;
}


// Debug
void Request::print_struct() const {
	std::cout << "===== Here is the request struct =====" << std::endl;
	std::cout << _parsed_request.method << " "
			  << _parsed_request.uri << " "
			  << _parsed_request.http_version << "\n";

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