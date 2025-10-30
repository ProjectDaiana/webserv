#include "helper/multipart_utils.hpp"

#include "CGI.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/wait.h>

CGI::~CGI() {
    if (_running && _pid > 0) {
        kill(_pid, SIGTERM);
        waitpid(_pid, NULL, WNOHANG);
    }
    
    if (_stdout_fd >= 0) {
        close(_stdout_fd);
    }
}

char** CGI::build_envp(const std::string& method,
                      const std::string& http_version,
					  const std::string& content_length,
					  const std::string& content_type) {
    _env_storage.clear();
    _env_ptrs.clear();
    
    _env_storage.push_back("REQUEST_METHOD=" + method);
    _env_storage.push_back("SCRIPT_FILENAME=" + get_script_filename());   // full path
    _env_storage.push_back("SCRIPT_NAME=" + get_script_name());        // URI path
    _env_storage.push_back("DOCUMENT_ROOT=" + get_document_root());
    _env_storage.push_back("SERVER_PROTOCOL=" + http_version);
	_env_storage.push_back("CONTENT_LENGTH=" + content_length);
	_env_storage.push_back("CONTENT_TYPE=" + content_type);
    
    for (std::vector<std::string>::iterator it = _env_storage.begin();
         it != _env_storage.end(); ++it) {
        _env_ptrs.push_back(const_cast<char*>(it->c_str()));
    }
    _env_ptrs.push_back(NULL); // execve expects c strings
    return &_env_ptrs[0];
}


// Extract and save uploaded file from multipart/form-data body
bool CGI::extract_and_save_uploaded_file(const std::string& body, const std::string& boundary, const std::string& out_filename) {
    fprintf(stderr, "[CGI] extract_and_save_uploaded_file called: body_size=%zu, boundary='%s', out='%s'\n",
            body.size(), boundary.c_str(), out_filename.c_str());
    size_t show = std::min<size_t>(body.size(), 200);
    fprintf(stderr, "[CGI] body snippet:\n%.*s\n", (int)show, body.c_str());
    bool ok = extract_and_save_multipart_file(body, boundary, out_filename);
    if (ok) {
        set_uploaded_file_path(out_filename);
        fprintf(stderr, "[CGI] extraction succeeded, saved to '%s'\n", out_filename.c_str());
    } else {
        fprintf(stderr, "[CGI] extraction failed\n");
    }
    return ok;
}