#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <vector>
#include <ctime>

class Client;
typedef struct s_location t_location;

class CGI {
private:
	pid_t _pid;
    time_t _start_time;
	bool _running;
	bool _writing;
	int _written;
    std::string _output;
	int _stdout_fd;
	int _stdin_fd;
	int _error_fd;
    std::vector<std::string> _env_storage;
    std::vector<char*> _env_ptrs;
	std::string _script_path;     // Absolute filesystem path to CGI script
    std::string _script_name;     // URI path as requested by client
    std::string _document_root;   // Root directory from which script is served
    std::string _interpreter;     // Path to interpreter (e.g., /usr/bin/python3)
	t_location* _location;

	public:
	CGI() : _pid(-1), 
			_start_time(0),
			_running(false), 
			_writing(false),
			_written(0),
			_output(""), 
			_stdout_fd(-1), 
			_stdin_fd(-1), 
			_error_fd(-1),
			_script_path(""),
			_script_name(""),
			_document_root(""),
			_interpreter(""),
			_location(NULL) {}
    ~CGI();
    
    void reset() {
		_pid = -1;
        _running = false;
        _output.clear();
        _start_time = 0;
        _stdout_fd = -1;
		_stdin_fd = -1;
		_error_fd = -1;
        _env_storage.clear();
        _env_ptrs.clear();
    }
    
    void prepare_paths(const std::string& script_path,
		const std::string& script_name,
		const std::string& document_root,
		const std::string& interpreter) {
        _script_path = script_path;
        _script_name = script_name;
        _document_root = document_root;
        _interpreter = interpreter;
    }
	
	char** build_envp(const std::string& method,
                      const std::string& http_version,
					  const std::string& content_length,
					  const std::string& content_type);
					  
	bool parse_multipart(Client& client);

	// Getters for run_cgi 
	char** get_envp();
	const std::string& get_script_path() const { return _script_path; }
    const std::string& get_script_name() const { return _script_name; }
    const std::string& get_document_root() const { return _document_root; }
    const std::string& get_interpreter() const { return _interpreter; }
	int get_stdout() const { return _stdout_fd; }
	int get_stdin() const { return _stdin_fd; }
	
    // Getters
    const std::string& get_output() const { return _output; }
    pid_t get_pid() const { return _pid; }
    time_t get_start_time() const { return _start_time; }
	int  get_written() const {return _written; }
    std::string get_cgi_upload_store() const;
	t_location* get_location() { return _location; }
	
	//Setters
	void set_stdout(int fd) { _stdout_fd = fd; }
	void set_stdin(int fd) { _stdin_fd = fd; }
	void set_error_fd(int fd) { _error_fd = fd; }
    void set_output(const std::string& output) { _output = output; }
    void set_pid(pid_t pid) { _pid = pid; }
    void set_start_time() { _start_time = std::time(NULL); }
    void set_running(bool running) { _running = running; }
	void set_writing(bool writing) { _writing = writing; }
    void set_written(int n) { _written = n; }
	bool is_running() const { return _running; }
	bool is_writing() const { return _writing; }
	int get_error_fd() const { return _error_fd; }
	void set_location(t_location* loc) { _location = loc; }

    // Extract and save uploaded file from multipart/form-data body
    // Returns true on success, false on failure
    bool extract_and_save_uploaded_file(const std::string& body, const std::string& boundary, const std::string& out_filename);
};
#endif