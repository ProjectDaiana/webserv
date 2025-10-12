#pragma once
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <errno.h>
#include <signal.h>

bool run_cgi(const std::string& cgi_path, const std::string& built_path, Client& client, std::vector<struct pollfd>& pfds) {
	int pipefd_out[2]; // To Server
    int pipefd_in[2]; // From Server
    pipe(pipefd_out);
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

		char** envp = client.build_cgi_envp(built_path); // headers need to go here
		char* const argv[] = {
			const_cast<char*>(cgi_path.c_str()), // Interpreter (e.g., python3)
			const_cast<char*>(built_path.c_str()), // www/cgi-bin/test.py
			NULL
		};
        execve(cgi_path.c_str(), argv, envp);

        _exit(1); //TODO replace, exit not allowed
    }
	printf("=== CGI After execvefd '%d' is being added to poll\n \n\n", pipefd_out[0]);
    close(pipefd_out[1]);
	close(pipefd_in[0]);
	pfds.push_back(Server::create_pollfd(pipefd_out[0], POLLIN, 0)); //POLLIN
//    cgi_pipes[pipefd_out[0]] = &client;
	client.set_cgi_pipe_fd(pipefd_out[0]);
	client.set_cgi_pipe_fd(pipefd_out[1]);
    client.set_cgi_pid(pid);

//	if (client.is_post_req() == 'POST' && client.has_request_body()) {
    //     pfds.push_back(Server::create_pollfd(pipefd_in[1], POLLOUT, 0));
    // } else {
    //     // No body to send, close stdin immediately
    //     close(pipefd_in[1]);
    //     client.set_cgi_stdin_fd(-1);
    // }

	return true; 
}

bool cgi_eof(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds)
{
	printf("=== CGI pipe closed (EOF), writing response =====================\n");

        // Save PID before resetting it!
        pid_t cgi_pid = client.get_cgi_pid();
        // Reset client CGI state
        client.set_cgi_running(0);
        //client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);

        // Close pipe and wait for child
        close(pipe_fd);
		// IMPORTANT: Remove fd from pfds vector
			for (size_t i = 0; i < pfds.size(); i++) {
				if (pfds[i].fd == pipe_fd) {
					printf("Removing pipe fd %d from pfds\n", pipe_fd);
					pfds.erase(pfds.begin() + i);
					break;
				}
			}


        std::cout << "pipde fd "<< client.get_cgi_pipe() << std::endl;

        int ret_pid = waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!
        printf("=== CGI complete output: pid %d,  %s =====================\n", ret_pid, client.cgi_output.c_str());
        return true;  // CGI finished
}

bool handle_cgi_read_from_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
    char buf[10];
    ssize_t n;
    
    printf("=== handle_cgi_read_from_pipe called for pipe fd %d =====================\n", pipe_fd);
    n = read(pipe_fd, buf, sizeof(buf) - 1);
    
    if (n > 0) {
        printf("=== CGI Reading from pipe, got %zd bytes =====================\n", n);
        // Null terminate for debug output
        buf[n] = '\0';
        printf("=== CGI output chunk: %s =====================\n", buf);
        client.cgi_output.append(buf, n);
        return false;  // Keep reading
    }

    if (n == 0) 
        return cgi_eof(pipe_fd, client, pfds);  // CGI finished
    
    if (n < 0) {
        // Error occurred
        printf("=== CGI read error: %s =====================\n", strerror(errno));
        
        // Save PID before resetting
        pid_t cgi_pid = client.get_cgi_pid();
        
        client.set_cgi_running(0);
        //client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);
        
        close(pipe_fd);
        waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID
        return true;
    }
    
    return false;
}

bool check_cgi_timeout(Client& client, int timeout) {
	printf("\nCGI TIMEOUT CHECK NOW!\n");
	if (!client.is_cgi_running()) {
		return false;  // Not running CGI
    }
    
    time_t now = std::time(NULL);
    time_t elapsed = now - client.get_cgi_start_time();
    std::cout << "XXXXXXXX cgi start time: " <<  client.get_cgi_start_time() << std::endl;
	//pid_t cgi_pid = client.get_cgi_pid();
    
		printf("time elapsed: '%lld'\n", (long long)elapsed);
		printf("cgi start time: '%lld'\n", (long long)client.get_cgi_start_time());
		printf("timeout: '%d'\n", timeout);
		printf("client fd is: '%d'\n", client.get_fd());
    if (elapsed > timeout) {
        std::cout << "XXXXXXXXXX CGI Timeout" << std::endl;
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
        //cleanup_cgi_process(client, pipe_fd, true);  // Sets 504 response
        return true;
    }
//    return true; // Trigger timeout
    return false;
}

bool handle_cgi_timeout(Client& client) {
    const int CGI_TIMEOUT = 0;           
    if (!client.is_cgi_running()) {
        std::cout << "XXXXX No cgi running" << std::endl;
        return false;
    }

	if (check_cgi_timeout(client, CGI_TIMEOUT)) {
        //TODO this is hardcoded
        // client.cgi_output = "HTTP/1.1 504 Gateway Timeout\r\n"
        //                    "Content-Type: text/html\r\n"
        //                    "Content-Length: 54\r\n"
        //                    "\r\n"
        //                    "<html><body><h1>504 Gateway Timeout</h1></body></html>"; //TODO overwrite cgi buffer so this gets handled in the response

        client.set_error_code(504);
        //client.set_cgi_running(0);
        //client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);
        client.set_cgi_start_time();

        return true;  // Timeout occurred
    }
    return false;  // No timeout
}


bool handle_cgi_write_to_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
	char buf[10]; // from a buffer or from raw_request or client.get_body();
    ssize_t n;
    
    printf("=== handle_cgi_write_to_pipe called for pipe fd %d =====================\n", pipe_fd);
    //n = read(pipe_fd, buf, sizeof(buf) - 1);
	std::string body = client.get_body();
	int n = write(pipe_fd, body.c_str(), sizeof(buf));
	int written = client.written_to_cgi + n; //restet this variable after writting to pipe is done;
	int content_len = atoi(client.get_header("Content-Length").c_str());
    if (n < content_len) {
        printf("=== CGI writting to pipe %zd bytes =====================\n", n);
        // Null terminate for debug output
        //buf[n] = '\0';
        printf("=== CGI input chunk: %s =====================\n", buf);
        client.cgi_input.append(buf, n);
        return false;  // Keep writting

    if (n == content_len) {
		printf("=== CGI pipe in closed (done writting), writing response =====================\n");

        // Save PID before resetting it!
        pid_t cgi_pid = client.get_cgi_pid();
        // Reset client CGI state
        client.set_cgi_running(0);
        //client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);

        // Close pipe and wait for child
        close(pipe_fd);
		// IMPORTANT: Remove fd from pfds vector
			for (size_t i = 0; i < pfds.size(); i++) {
				if (pfds[i].fd == pipe_fd) {
					printf("Removing pipe fd %d from pfds\n", pipe_fd);
					pfds.erase(pfds.begin() + i);
					break;
				}
			}
        //std::cout << "pipde fd "<< client.get_cgi_pipe() << std::endl;

        int ret_pid = waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!
        printf("=== CGI complete output: pid %d,  %s =====================\n", ret_pid, client.cgi_input.c_str());
        return true;  // CGI finished
	}

    
    if (n > 0) {
        // Error occurred
        printf("=== CGI writting out of bounds: %s =====================\n", strerror(errno));
        
        // Save PID before resetting
        pid_t cgi_pid = client.get_cgi_pid();
        
        client.set_cgi_running(0);
        //client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);
        
        close(pipe_fd);
        waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID
        return true;
    }
    
    return false;
}