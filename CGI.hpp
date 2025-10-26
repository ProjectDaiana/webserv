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
	bool _writing;
    std::string _output;
    time_t _start_time;
	int _stdout_fd;
	int _stdin_fd;
    std::vector<std::string> _env_storage;
    std::vector<char*> _env_ptrs;
	int _written;

	std::string _script_filename; // full FS path
    std::string _script_name;     // URI
    std::string _document_root;   // root used to build script
    std::string _interpreter;
    
public:
	CGI() : _pid(-1), _running(false), _output(""), _start_time(0), _stdout_fd(-1), _stdin_fd(-1), _written(0) {}
    ~CGI();
    
    void reset() {
        _pid = -1;
        _running = false;
        _output.clear();
        _start_time = 0;
        _stdout_fd = -1;
		_stdin_fd = -1;
        _env_storage.clear();
        _env_ptrs.clear();
    }
    
    void prepare_paths(const std::string& script_filename,
                 const std::string& script_name,
                 const std::string& document_root,
                 const std::string& interpreter) {
        _script_filename = script_filename;
        _script_name = script_name;
        _document_root = document_root;
        _interpreter = interpreter;
    }
	
	char** build_envp(const std::string& method,
                      const std::string& http_version,
					  const std::string& content_length,
					  const std::string& content_type);


	// getters for run_cgi 
	char** get_envp();
    const std::string& get_script_filename() const { return _script_filename; }
    const std::string& get_script_name()     const { return _script_name; }
    const std::string& get_document_root()   const { return _document_root; }
    const std::string& get_interpreter()     const { return _interpreter; }
	int get_stdout() const { return _stdout_fd; }
	int get_stdin() const { return _stdin_fd; }

    // Getters Setters
    const std::string& get_output() const { return _output; }
    pid_t get_pid() const { return _pid; }
    time_t get_start_time() const { return _start_time; }
	int  get_written() const {return _written; }

	void set_stdout(int fd) { _stdout_fd = fd; }
	void set_stdin(int fd) { _stdin_fd = fd; }
    void set_output(const std::string& output) { _output = output; }
    void set_pid(pid_t pid) { _pid = pid; }
    void set_start_time() { _start_time = std::time(NULL); }
    void set_running(bool running) { _running = running; }
	void set_writing(bool writing) { _writing = writing; }
    void set_written(int n) { _written = n; }
	bool is_running() const { return _running; }
	bool is_writing() const { return _writing; }

};
#endif