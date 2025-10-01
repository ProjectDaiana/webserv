#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Request.hpp"
#include "webserv.hpp"
#include "Server.hpp"

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
		int cgi_pipe_fd;

		Server *_server;
		Request _request;

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
		const std::map<std::string, std::string>& get_headers() const;
		const std::string& get_header(const std::string& key) const;
		const std::string& get_body() const;
		const s_error& get_parse_error() const;
		int get_fd() const;
		time_t get_last_activity() const;
		const Server *get_server() const {return _server;}
		//bool is_cgi_done() const { return _cgi_done; }
		// Inside Client class

//TODO seperate debug from setters
		// Debug
		void print_raw_request() const;
		int	get_error_code() const;
		void set_error_code(int code);
		const t_request& get_request() const;
		void set_request(const t_request& new_request);
		void print_request_struct() const;
  
		void set_write_complete(bool value) { _write_complete = value; }
		void set_keep_alive(bool value) { _keep_alive = value; }
		bool get_keep_alive() const { return _keep_alive; }
		//void set_cgi_done(bool done) { _cgi_done = done; }

		//CGI
		pid_t cgi_pid;
		bool cgi_running;
		std::string cgi_output;

		void set_cgi_output(const std::string& output) {
			cgi_output = output;
		}
		const std::string& get_cgi_output() const {
        	return cgi_output;
    	}

		void set_cgi_pipe_fd(int fd) {
			cgi_pipe_fd = fd;
		};
		void set_cgi_pid(pid_t pid) {
			cgi_pid = pid;
		};

		bool is_cgi_running() {

			printf("CGI IS RUNNING?: '%d'\n", (int)cgi_running);
			return cgi_running;
		}
		void set_cgi_running(bool b) {
			printf("CGI IS BEING SET TO: '%d'\n", (int)b);
			cgi_running = b;
		}

		pid_t get_cgi_pid () {
			return cgi_pid;
		}

		int get_cgi_pipe() const {
			return cgi_pipe_fd;
		}
};

#endif
