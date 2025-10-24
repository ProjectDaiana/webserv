#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Request.hpp"
#include "webserv.hpp"
#include "Server.hpp"
#include "CGI.hpp"

class Server;

class Client {
	private:
		int _fd;
		std::string _raw_request;
		bool _headers_complete;
		bool _read_complete;
		bool _write_complete;
		//bool _cgi_done;
		bool _is_parsed;
		bool _keep_alive;
		size_t _content_len;
		size_t _headers_end_pos;
		time_t _last_activity; // Track last activity for timeout
		int _error_code; //TODO change back to 200 when reset
		int cgi_stdout_fd;
		int cgi_stdin_fd;

		Server *_server;
		Request _request;
		CGI _cgi;

	public:
		Client();
		Client(int fd, Server &server);
		~Client();

		void update_activity();
		void add_to_request(char* data, int len);
		bool parse_request();
		void reset();
		
		// Getters
		bool is_read_complete() const;
		bool is_headers_complete() const;
		bool is_cgi() const;
		bool is_write_complete() const { return _write_complete; }
		std::string& get_raw_request();
		size_t get_headers_end_pos() { return _headers_end_pos; };
		const std::string& get_method() const;
		const std::string& get_uri() const;
		const std::string& get_path() const;
		const std::string& get_query() const;
		const std::string& get_http_version() const;
		const std::map<std::string, std::string>& get_headers() const;
		const std::string& get_header(const std::string& key) const;
		const std::string& get_body() const;
		const s_error& get_parse_error() const;
		int get_fd() const;
		time_t get_last_activity() const;
		const Server *get_server() const {return _server;}
		int	get_error_code() const;
		bool get_keep_alive() const { return _keep_alive; }
		const t_request& get_request() const;
		
		//Setters
		void set_error_code(int code);
		void set_keep_alive(bool value) { _keep_alive = value; }
		void set_request(const t_request& new_request);
		void set_write_complete(bool value) { _write_complete = value; }
		//void set_cgi_done(bool done) { _cgi_done = done; }

		//CGI
		pid_t cgi_pid;
		bool cgi_running;
		std::string cgi_output;
		time_t cgi_start_time;

		bool is_cgi_running() {
			// printf("CGI IS RUNNING?: '%d'\n", (int)cgi_running);
			return cgi_running;
		}
		bool is_cgi_writing() {  return _cgi.is_writing(); }

		// CGI Setters
		void set_cgi_stdout_fd(int fd) { cgi_stdout_fd = fd; };
		void set_cgi_stdin_fd(int fd) { cgi_stdin_fd = fd; }
		void set_cgi_pid(pid_t pid) { cgi_pid = pid; };		
		void set_cgi_start_time() { cgi_start_time = std::time(NULL); }
		void set_cgi_running(bool b) {
			printf("CGI IS BEING SET TO: '%d'\n", (int)b);
			cgi_running = b;
		}
		void set_cgi_writing(bool b) {
			printf("CGI Writting: '%d'\n", (int)b);
			_cgi.set_writing(b);
		}
		void set_cgi_written(int n) { _cgi.set_written(n); }
		
		// CGI Getters
		CGI& get_cgi() { return _cgi; }
		pid_t get_cgi_pid () { return cgi_pid; }
		int get_cgi_stdout_fd() const { return cgi_stdout_fd; }
		int get_cgi_stdin_fd() const { return cgi_stdin_fd; }
		time_t get_cgi_start_time() { return cgi_start_time; }
		int get_cgi_written() { return _cgi.get_written();}

		
		// Debug
		void print_raw_request() const;
		void print_request_struct() const;
};

#endif
