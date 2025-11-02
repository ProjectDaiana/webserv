#pragma once
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <errno.h>
#include <signal.h>
#include <fcntl.h> ///
#include "webserv.hpp"

bool run_cgi(Client& client, std::vector<struct pollfd>& pfds)
{
	int pipefd_out[2]; // To Server
    int pipefd_in[2]; // From Client
    pipe(pipefd_out);
    pipe(pipefd_in);

    // make server-side ends non-blocking:
    // pipefd_out[0] -> server reads CGI stdout
    // pipefd_in[1]  -> server writes to CGI stdin
    // int flags = fcntl(pipefd_out[0], F_GETFL, 0);
    // if (flags != -1) fcntl(pipefd_out[0], F_SETFL, flags | O_NONBLOCK);
    // flags = fcntl(pipefd_in[1], F_GETFL, 0);
    // if (flags != -1) fcntl(pipefd_in[1], F_SETFL, flags | O_NONBLOCK);

	// printf("[CGI DEBUG] pipefd_out[0] (read end, server reads): %d\n", pipefd_out[0]);
	// printf("[CGI DEBUG] pipefd_out[1] (write end, CGI writes): %d\n", pipefd_out[1]);
	// printf("[CGI DEBUG] pipefd_in[0] (read end, CGI reads): %d\n", pipefd_in[0]);
	// printf("[CGI DEBUG] pipefd_in[1] (write end, server writes): %d\n", pipefd_in[1]);
    printf("=== CGI will run for Client %d ===================== \n\n", client.get_fd());
    pid_t pid = fork();

    client.set_cgi_start_time();
	client.set_cgi_running(1);
    if (pid == 0) {
        dup2(pipefd_out[1], STDOUT_FILENO);
		dup2(pipefd_in[0], STDIN_FILENO);
        close(pipefd_out[0]);
        close(pipefd_out[1]);
        close(pipefd_in[0]);
        close(pipefd_in[1]);

        std::string abs_script = client.get_cgi().get_script_filename();   // already resolved by validate_and_resolve_path
        std::string docroot    = client.get_cgi().get_document_root();

		size_t slash = abs_script.find_last_of('/');
		std::string script_dir = ".";
		std::string script_base = abs_script;
		if (slash != std::string::npos) {
			script_dir = abs_script.substr(0, slash);         // "www/html/cgi-bin"
			script_base = abs_script.substr(slash + 1);       // "test.py"
			if (chdir(script_dir.c_str()) != 0) {
				perror("chdir");
				_exit(127);
			}
		}

        char ** envp = client.get_cgi().build_envp(
            client.get_method(),
            client.get_http_version(),
            client.get_header("Content-Length"),
            client.get_header("Content-Type")
        );
    

		// build argv: interpreter then script path
        const std::string& interp = client.get_cgi().get_interpreter();
        char* const argv[] = {
            const_cast<char*>(interp.c_str()),
            const_cast<char*>(script_base.c_str()),
            NULL
        };

		// fprintf(stderr, "[run_cgi] interpreter='%s', script_filename='%s'\n", interp.c_str(), abs_script.c_str());
        execve(interp.c_str(), argv, envp);
        perror("execve");
        _exit(127);
    }
	printf("=== CGI After execvefd '%d' is being added to poll\n \n\n", pipefd_out[0]);
    close(pipefd_out[1]);
	// Parent doesn't need the read end of the CGI stdin pipe
	close(pipefd_in[0]);
	pfds.push_back(Server::create_pollfd(pipefd_out[0], POLLIN, 0)); //POLLIN
//    cgi_pipes[pipefd_out[0]] = &client;
	client.set_cgi_stdout_fd(pipefd_out[0]);
	client.set_cgi_written(0);
    client.set_cgi_pid(pid);

	if (client.get_method() == "POST" && !client.get_body().empty()) {
		//printf("=== POST request with %zu bytes body =====================\n", client.get_body().length());

		//printf("\033[33mrunCGI: cgi boddy %s\033[0m\n", client.get_body().c_str());
		//	printf("\033[33m POST Content Type %s =====================\n", client.get_header("Content-Type").c_str());
	    client.set_cgi_writing(1);
		client.set_cgi_stdin_fd(pipefd_in[1]);
        pfds.push_back(Server::create_pollfd(pipefd_in[1], POLLOUT, 0));
    } else {
        // No body to send, close stdin immediately
		printf("NO PIPE STDIN\n");
        close(pipefd_in[1]);
        client.set_cgi_stdin_fd(-1);
		client.set_cgi_writing(0);
    }

	return true; 
}

bool cgi_eof(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds)
{
	// printf("=== CGI pipe closed (EOF), writing response =====================\n");
	// printf("\033[33mDEBUG BEFORE EOF: client_fd=%d, cgi_stdin_fd=%d, cgi_written=%d, body_len=%zu, content-length='%s'\033[0m\n",
	// client.get_fd(),
	// client.get_cgi_stdin_fd(),
	// client.get_cgi_written(),
	// client.get_body().size(),
	// client.get_header("Content-Length").c_str());
	// printf("\033[33m POST Content Type %s =====================\n", client.get_header("Content-Type").c_str());
	// printf("\033[33mrunCGI: cgi body %s\033[0m\n", client.get_body().c_str());
        
	// Save PID before resetting it!
	pid_t cgi_pid = client.get_cgi_pid();
	client.set_cgi_running(0);
	client.set_cgi_pid(-1);

	if(client.get_method() == "POST")
    	client.get_cgi().parse_multipart(client);
	close(pipe_fd);
	std::cout << "closed fd "<< pipe_fd << std::endl;

	// IMPORTANT: Remove fd from pfds vector
	for (size_t i = 0; i < pfds.size(); i++) {
		if (pfds[i].fd == pipe_fd) {
			printf("Removing pipe fd %d from pfds\n", pipe_fd);
			pfds.erase(pfds.begin() + i);
			break;
		}
	}

	waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!
	// printf("=== CGI complete output: pid %d,  %s END=====================\n", ret_pid, client.cgi_output.c_str());

	return true;  // CGI finished
}

bool handle_cgi_read_from_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
    char buf[1000];
    ssize_t n;
    
	//printf("\033[32m=== handle_cgi_read_from_pipe called for pipe fd %d =====================\033[0m\n", pipe_fd);
    n = read(pipe_fd, buf, sizeof(buf) - 1);
    
    if (n > 0) {
    	//fprintf(stderr, "=== CGI output chunk(%zd bytes): %s =====================\n",n, buf); //OJO this printf connflicts with buffer and will break everything
        client.cgi_output.append(buf, n);
        return false;  // Keep reading
    }

    if (n == 0)
		return cgi_eof(pipe_fd, client, pfds);  // CGI finished

    if (n < 0) {
        // Error occurred
		printf("\033[31m=== CGI read error: %s =====================\033[0m\n", strerror(errno));
        
        // Save PID before resetting
        pid_t cgi_pid = client.get_cgi_pid();
        
        client.set_cgi_running(0);
        //client.set_cgi_stdout_fd(-1);
        client.set_cgi_pid(-1);
        
        close(pipe_fd);
        waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID
        return true;
    }
    return false;
}

bool check_cgi_timeout(Client& client, int timeout) {
	if (!client.is_cgi_running()) {
		return false;  // Not running CGI
    }
    time_t now = std::time(NULL);
    time_t elapsed = now - client.get_cgi_start_time();

    if (elapsed > timeout) {
		// std::cout << "XXXXXXXXXX CGI Timeout" << std::endl;
		// printf("time elapsed: '%lld'\n", (long long)elapsed);
		// printf("cgi start time: '%lld'\n", (long long)client.get_cgi_start_time());
		// printf("timeout: '%d'\n", timeout);
		// printf("client fd is: '%d'\n", client.get_fd());
        pid_t cgi_pid = client.get_cgi_pid();
        if (cgi_pid > 0) {
            kill(cgi_pid, SIGTERM);
            usleep(100000);

            if (kill(cgi_pid, 0) == 0) {
                kill(cgi_pid, SIGKILL);   // Force kill
            }
        }

		if (cgi_pid > 0) {
            kill(cgi_pid, SIGTERM);  // Try graceful termination
            usleep(100000);
            
            // Force kill if still alive
            if (kill(cgi_pid, 0) == 0) {
                kill(cgi_pid, SIGKILL);
            }
            waitpid(cgi_pid, NULL, WNOHANG);
        }
        return true;
    }
//    return true; // Trigger timeout
    return false;
}

bool handle_cgi_timeout(Client& client) {
    if (!client.is_cgi_running()) {
        std::cout << "XXXXX No cgi running" << std::endl;
        return false;
    }

    if (check_cgi_timeout(client, CGI_TIMEOUT)) {
        client.set_error_code(504);
        client.set_cgi_running(0);
        client.set_cgi_pid(-1);
        client.set_cgi_start_time();
        return true;  // Timeout occurred
    }
    return false;  // No timeout
}


bool handle_cgi_write_to_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
//    printf("=== handle_cgi_write_to_pipe called for pipe fd %d =====================\n", pipe_fd);
    const std::string body = client.get_body();
    ssize_t body_len = static_cast<ssize_t>(body.length());
    ssize_t written = static_cast<ssize_t>(client.get_cgi_written());
    ssize_t remaining = (written < body_len) ? body_len - written : 0;

//    printf("DEBUG: Content-Length header? %s, body_len=%zd, written=%zd, remaining=%zd\n",
//           client.get_header("Content-Length").c_str(), body_len, written, remaining);

    if (remaining == 0) {
        // nothing to write — cleanup this pipe
    //    printf("=== CGI pipe in closed (done writing), cleaning up fd %d =====================\n", pipe_fd);
        for (size_t i = 0; i < pfds.size(); i++) {
            if (pfds[i].fd == pipe_fd) { pfds.erase(pfds.begin() + i); break; }
        }
        close(pipe_fd);
        client.set_cgi_writing(0);
        client.set_cgi_written(0);
        client.set_cgi_stdin_fd(-1);
        return true;
    }

    // write from the actual body buffer (single non-blocking write attempt)
    const char *buf = body.c_str() + written;
    ssize_t n = write(pipe_fd, buf, static_cast<size_t>(remaining));

    if (n > 0) {
        written += n;
        client.set_cgi_written(static_cast<int>(written));
    //    printf("=== Wrote %zd bytes to CGI stdin (%zd/%zd total) =====================\n", n, written, body_len);

        if (written == body_len) {
            // all bytes written -> cleanup
	//		printf("\033[32m=== All POST body written (%zd/%zd), pipe %d =====================\033[0m\n", written, body_len, pipe_fd);
            for (size_t i = 0; i < pfds.size(); i++) {
                if (pfds[i].fd == pipe_fd) { pfds.erase(pfds.begin() + i); break; }
            }
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
    }

    return false;
}