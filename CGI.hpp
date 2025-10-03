#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>
#include <ctime>

class Client;

class CGI {
private:
	pid_t _pid;
	bool _running;
    std::string _output;
    time_t _start_time;
	int _pipe_fd;
    std::vector<std::string> _env_storage;
    std::vector<char*> _env_ptrs;
    
public:
	CGI() : _pid(-1), _running(false), _output(""), _start_time(0), _pipe_fd(-1) {}
    ~CGI();
    
    void reset() {
        _pid = -1;
        _running = false;
        _output.clear();
        _start_time = 0;
        _pipe_fd = -1;
        _env_storage.clear();
        _env_ptrs.clear();
    }
    
    // Getters Setters
    const std::string& get_output() const { return _output; }
    pid_t get_pid() const { return _pid; }
    int get_pipe_fd() const { return _pipe_fd; }
    time_t get_start_time() const { return _start_time; }

    void set_output(const std::string& output) { _output = output; }
    void set_pipe_fd(int fd) { _pipe_fd = fd; }
    void set_pid(pid_t pid) { _pid = pid; }
    void set_start_time() { _start_time = std::time(NULL); }
    void set_running(bool running) { _running = running; }
    bool is_running() const { return _running; }
    
	char** build_envp(const std::string& script_path, 
						const std::string& method,
						const std::string& http_version);

    //static int validate_path(const std::string& request_path, const t_server& config,
    //                       std::string& built_path, std::string& cgi_path);
    
};
#endif