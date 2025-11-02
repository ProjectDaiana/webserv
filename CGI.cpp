#include "webserv.hpp"
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


bool CGI::parse_multipart(Client& client)
{
    const std::string& content_type = client.get_header("Content-Type");
    if (content_type.find("multipart/form-data") == std::string::npos)
		return false;

	std::string boundary = extract_boundary_from_disposition(content_type);
    std::string upload_dir = get_cgi_upload_store();
    std::string body = client.get_body();
    std::string filename;
    size_t disp_pos = body.find("Content-Disposition: form-data;");
    while (disp_pos != std::string::npos) {
        size_t header_end = body.find("\r\n\r\n", disp_pos);
        if (header_end == std::string::npos)
            header_end = body.find("\n\n", disp_pos);
        if (header_end == std::string::npos)
            header_end = body.size();
        std::string disp_block = body.substr(disp_pos, header_end - disp_pos);

        if (disp_block.find("filename=\"") != std::string::npos) {
            filename = extract_filename_from_disposition(disp_block);
            // fprintf(stderr, "[DEBUG] filename=%s\n", filename.c_str());
            break;
        }
        disp_pos = body.find("Content-Disposition: form-data;", header_end);
    }

    if (filename.empty()) {
        //fprintf(stderr, "[ERROR] No filename found in multipart headers!\n");
        return false;
    }

	std::string unique_filename = make_unique_filename(filename);
    std::string out_filename = upload_dir + "/" + unique_filename;
    //set_uploaded_file_path(out_filename);

    if (extract_and_save_multipart_file(body, boundary, out_filename)) {
        //set_uploaded_file_path(out_filename);
        return true;
     } else {
    //     set_uploaded_file_ext("");
         return false;
    }
}


// std::string CGI::get_cgi_upload_store() const {
//     return _location ? _location->cgi_upload_store : "";
// }