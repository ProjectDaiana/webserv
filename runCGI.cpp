
#include "runCGI.hpp"

void remove_fd_from_poll(int fd, std::vector<struct pollfd>& pfds) {
	for (size_t i = 0; i < pfds.size(); i++) {
		if (pfds[i].fd == fd) {
			pfds.erase(pfds.begin() + i);
			break;
		}
	}
}

int run_cgi(Client& client, std::vector<struct pollfd>& pfds)
{
	int pipefd_out[2];  // CGI stdout -> Server
    int pipefd_in[2];   // Server -> CGI stdin
    int pipefd_err[2];  // CGI error codes -> Server

    if (pipe(pipefd_out) == -1 || pipe(pipefd_in) == -1 || pipe(pipefd_err) == -1) {
        if (pipefd_out[0] >= 0) { close(pipefd_out[0]); close(pipefd_out[1]); }
        if (pipefd_in[0] >= 0) { close(pipefd_in[0]); close(pipefd_in[1]); }
        if (pipefd_err[0] >= 0) { close(pipefd_err[0]); close(pipefd_err[1]); }
        return 500; // Internal Server Error - pipe failed
    }
    
	pid_t pid = fork();
    if (pid == -1) {
        close(pipefd_out[0]); close(pipefd_out[1]);
        close(pipefd_in[0]); close(pipefd_in[1]);
        close(pipefd_err[0]); close(pipefd_err[1]);
        return 500; // Internal Server Error - fork failed
    }
    
    client.set_cgi_start_time();
	client.set_cgi_running(1);

    if (pid == 0) {
        // Child process
        dup2(pipefd_out[1], STDOUT_FILENO);
		dup2(pipefd_in[0], STDIN_FILENO);
        close(pipefd_out[0]);
        close(pipefd_out[1]);
        close(pipefd_in[0]);
        close(pipefd_in[1]);
        close(pipefd_err[0]); // Child doesn't read from error pipe
        
        int error_fd = pipefd_err[1]; // Keep this to write error codes
        std::string abs_script = client.get_cgi().get_script_path();
		size_t slash_pos = abs_script.find_last_of('/');

		std::string script_base = abs_script;
		
		if (slash_pos != std::string::npos) {
			std::string script_dir = abs_script.substr(0, slash_pos);         // "www/html/cgi-bin"
			script_base = abs_script.substr(slash_pos + 1);       // "test.py"
			if (chdir(script_dir.c_str()) != 0) {
				perror("chdir");
				int error_code = 500; // Internal Server Error
				write(error_fd, &error_code, sizeof(int));
				close(error_fd);
				while (1); // Hang - parent will kill us
			}
		}

        char ** envp = client.get_cgi().build_envp(
            client.get_method(),
            client.get_http_version(),
            client.get_header("Content-Length"),
            client.get_header("Content-Type")
        );
    
        const std::string& interp = client.get_cgi().get_interpreter();
        char* const argv[] = {
            const_cast<char*>(interp.c_str()),
            const_cast<char*>(script_base.c_str()),
            NULL
        };
        execve(interp.c_str(), argv, envp);
    
        // execve failed - write error code to error pipe
        int error_code = 500; // Internal Server Error - execve failed (bad interpreter)
        write(error_fd, &error_code, sizeof(int));
        close(error_fd);
        while (1); // Hang - parent will kill us
    }
    
    // Parent process continues here
    close(pipefd_out[1]);
	close(pipefd_in[0]);
    close(pipefd_err[1]); // Parent doesn't write to error pipe
    
	pfds.push_back(Server::create_pollfd(pipefd_out[0], POLLIN, 0)); //POLLIN
	client.set_cgi_stdout_fd(pipefd_out[0]);
    client.get_cgi().set_error_fd(pipefd_err[0]); // Store error pipe fd in CGI object
	client.set_cgi_written(0);
    client.set_cgi_pid(pid);

	// Write body to CGI stdin for POST and DELETE methods
	if ((client.get_method() == "POST" || client.get_method() == "DELETE") && !client.get_body().empty()) {
	    client.set_cgi_writing(1);
		client.set_cgi_stdin_fd(pipefd_in[1]);
        pfds.push_back(Server::create_pollfd(pipefd_in[1], POLLOUT, 0));
    } else {
        close(pipefd_in[1]);
        client.set_cgi_stdin_fd(-1);
		client.set_cgi_writing(0);
    }

	return 0; // Success - no error
}

bool cgi_eof(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds)
{
	// Save PID before resetting it!
	pid_t cgi_pid = client.get_cgi_pid();
	client.set_cgi_running(0);
	client.set_cgi_pid(-1);

	int error_fd = client.get_cgi().get_error_fd();
	if (error_fd >= 0) {
		int error_code = 0;
		ssize_t n = read(error_fd, &error_code, sizeof(int));
		if (n == sizeof(int) && error_code != 0) {
			client.set_error_code(error_code);
			printf(CLR_RED "ERROR code in cgi_eof %d\n" CLR_RESET, error_code);
			client.cgi_output.clear(); // 
		}
		close(error_fd);
		client.get_cgi().set_error_fd(-1);
	}

	if(client.get_method() == "DELETE") {
		std::string file = client.cgi_output;
		size_t start = file.find_first_not_of(" \t\r\n");
		size_t end = file.find_last_not_of(" \t\r\n");
	
		if (start != std::string::npos && end != std::string::npos)
			file = file.substr(start, end - start + 1);		
		
		std::string file_path = client.get_cgi().get_cgi_upload_store() + "/" + file;
	//	printf(CLR_MAGENTA "Full file_path: '%s'\n" CLR_RESET, file_path.c_str());
		handle_delete(client, NULL, file_path);
	}

	if(client.get_method() == "POST")
    	client.get_cgi().parse_multipart(client);
	close(pipe_fd);
	remove_fd_from_poll(pipe_fd, pfds);

	if (cgi_pid > 0) {
		int status;
		pid_t result = waitpid(cgi_pid, &status, WNOHANG);
		if (result == -1) {
			perror("waitpid in cgi_eof");
		} else if (result > 0) {
			if (WIFSIGNALED(status) && (client.get_error_code() == 0 || client.get_error_code() == 200)) {
				client.set_error_code(502); // Bad Gateway - CGI script crashed
			}
		}
	}
  client.set_cgi_stdout_fd(-1);
  client.set_cgi_stdin_fd(-1);
  client.get_cgi().reset();
  return true;  // CGI finished
}

bool handle_cgi_read_from_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
    char buf[1000];
    ssize_t n;
    
    n = read(pipe_fd, buf, sizeof(buf) - 1);

	if (n > 0) {
        client.cgi_output.append(buf, n);
        return false;  // Keep reading
    }

    if (n == 0)
		return cgi_eof(pipe_fd, client, pfds);  // CGI finished

    if (n < 0) {
        pid_t cgi_pid = client.get_cgi_pid();
		client.set_error_code(502); 
        client.set_cgi_running(0);
        client.set_cgi_pid(-1);
        //client.set_cgi_stdout_fd(-1); //TODO check if we need this
        
        close(pipe_fd);
        waitpid(cgi_pid, NULL, WNOHANG);
        return true;
    }
    return false;
}

bool check_cgi_timeout(Client& client, int timeout) {
	if (!client.is_cgi_running()) {
		return false;
    }
    time_t now = std::time(NULL);
    time_t elapsed = now - client.get_cgi_start_time();

    if (elapsed > timeout) {
        pid_t cgi_pid = client.get_cgi_pid();
        if (cgi_pid > 0) {
            if (kill(cgi_pid, SIGTERM) == 0) {
                usleep(100000); // Wait 100ms
                
                if (kill(cgi_pid, 0) == 0) {
                    kill(cgi_pid, SIGKILL);
                }
            }
            waitpid(cgi_pid, NULL, WNOHANG);
        }
        return true;
    }
    return false;
}

bool handle_cgi_timeout(Client& client) {
    if (!client.is_cgi_running() || !client.is_cgi())
        return false;

    if (check_cgi_timeout(client, CGI_TIMEOUT)) {
        // IMPORTANT: Check error pipe BEFORE setting timeout error
        // Child may have written error (chdir/execve fail) then hung with while(1)
        int error_fd = client.get_cgi().get_error_fd();
        
		if (error_fd >= 0) {
            int error_code = 0;
            int n = read(error_fd, &error_code, sizeof(int));
            if (n == sizeof(int) && error_code != 0)
                client.set_error_code(error_code);
            else
                client.set_error_code(504);
            close(error_fd);
            client.get_cgi().set_error_fd(-1); // Mark as closed
        } else
            client.set_error_code(504);
        
		client.set_cgi_running(0);
        client.set_cgi_pid(-1);
        client.set_cgi_start_time();
        return true;  // Timeout occurred
    }
    return false;  // No timeout
}


bool handle_cgi_write_to_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
    const std::string body = client.get_body();
    ssize_t body_len = static_cast<ssize_t>(body.length());
    ssize_t written = static_cast<ssize_t>(client.get_cgi_written());
    ssize_t remaining = (written < body_len) ? body_len - written : 0;

    if (remaining == 0) {
        remove_fd_from_poll(pipe_fd, pfds);
        close(pipe_fd);
        client.set_cgi_writing(0);
        client.set_cgi_written(0);
        client.set_cgi_stdin_fd(-1);
        return true;
    }

    const char *buf = body.c_str() + written;
    ssize_t n = write(pipe_fd, buf, static_cast<size_t>(remaining));

    if (n > 0) {
        written += n;
        client.set_cgi_written(static_cast<int>(written));

        if (written == body_len) {
            remove_fd_from_poll(pipe_fd, pfds);
            close(pipe_fd);
            client.set_cgi_stdin_fd(-1);
            client.set_cgi_written(0);
            client.set_cgi_writing(0);
            return true;
        }
        // partial write, keep fd in poll for POLLOUT and return to be retried on next event
        int pfd_idx = find_pfd(pipe_fd, pfds);
        if (pfd_idx != -1)
			pfds[pfd_idx].events |= POLLOUT;
        return false;
    } else if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false; // Non-blocking write, try again later
        }
        // Pipe broken or other error - CGI died before reading all input
        client.set_error_code(502); // Bad Gateway
        remove_fd_from_poll(pipe_fd, pfds);
        close(pipe_fd);
        client.set_cgi_stdin_fd(-1);
        client.set_cgi_writing(0);
        return true;
    }
    return false;
}
