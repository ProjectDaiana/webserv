#pragma once
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <errno.h>
#include <fcntl.h>

bool run_cgi(const std::string& script_path, Client& client, std::vector<struct pollfd>& pfds, std::map<int, Client*>& cgi_pipes) {
    int pipefd[2];
    pipe(pipefd);
    pid_t pid = fork();
	client.set_cgi_start_time();
    if (pid == 0) {
        // ---- child ----
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

		fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

		//TODO get methods from config
        char* const envp[] = {
            (char*)"REQUEST_METHOD=GET",
            (char*)"SCRIPT_NAME=test.py",
            (char*)"SERVER_PROTOCOL=HTTP/1.1",
            NULL
        };

		//TODO take paths from struct
        char* const argv[] = {
            (char*)script_path.c_str(),
            NULL
        };

        execve(script_path.c_str(), argv, envp);
        _exit(1); // only runs if execve fails
    }
	printf("=== CGI After execve ===================== \n\n");
    close(pipefd[1]);
	pfds.push_back(Server::create_pollfd(pipefd[0], POLLIN, 0)); //POLLIN
    cgi_pipes[pipefd[0]] = &client;
	client.set_cgi_pipe_fd(pipefd[0]);
    client.set_cgi_pid(pid);
	client.set_cgi_running(1);

	return true; 
}

bool handle_cgi_write(int pipe_fd, Client &client) {
    char buf[10]; // small buf only for debuggin but later immplement standard cgi MAX
    ssize_t n;
    
    n = read(pipe_fd, buf, sizeof(buf));
    
    if (n > 0) {
        printf("=== CGI Reading from pipe, got %zd bytes =====================\n", n);
        // // Null terminate for debug output
        // buf[n] = '\0';
    //    printf("=== CGI output chunk: %s =====================\n", buf);
        client.cgi_output.append(buf, n);

		// char debug_buf[11];  // buf size + 1
        // memcpy(debug_buf, buf, n);
        // debug_buf[n] = '\0';
        // printf("=== CGI chunk: [%s] ===\n", debug_buf);
	
        return false;  // Keep reading
	}

    if (n == 0) {
        // Save PID before resetting it!
        pid_t cgi_pid = client.get_cgi_pid();

        // Close pipe and wait for child
        close(pipe_fd);
        std::cout << "pipde fd "<< client.get_cgi_pipe() << std::endl;
  		
		int status;
        int ret_pid = waitpid(cgi_pid, &status, WNOHANG);  // Use saved PID, not -1!

		if (ret_pid == 0) {
			// Child still running - wait for it (blocking)
			printf("=== Child %d still running after EOF, waiting... ===\n", cgi_pid);
			waitpid(cgi_pid, &status, 0);  // ✅ Block until child exits
		}

		// Reset client CGI state
        client.set_cgi_running(0);
        client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1); 
		
        printf("=== CGI complete output: pid %d,  %s =====================\n", ret_pid, client.cgi_output.c_str());

        return true;
    }
    
    if (n < 0) {
        // Error occurred
        printf("=== CGI read error: %s =====================\n", strerror(errno));
        
        // Save PID before resetting
        pid_t cgi_pid = client.get_cgi_pid();
        
        client.set_cgi_running(0);
        client.set_cgi_pipe_fd(-1);
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
	//pid_t cgi_pid = client.get_cgi_pid();
    
    if (elapsed > timeout) {
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
    
    return false;
}

// void handle_cgi_timeout(Client &client, int timeout) {
// 		        pid_t cgi_pid = client.get_cgi_pid();
//         int pipe_fd = client.get_cgi_pipe();
// 	// Check CGI timeout first
// 	if (check_cgi_timeout(*client, pfds[i].fd, timeout)) {
// 		// CGI timed out, remove from pfds and cgi_pipes
// 		cgi_pipes.erase(pfds[i].fd);
// 		pfds.erase(pfds.begin() + i);
// 		--i;
		
// 		// Re-enable the client socket for writing
// 		int client_fd = client->get_fd();
// 		for (size_t j = 0; j < pfds.size(); ++j) {
// 			if (pfds[j].fd == client_fd) {
// 				pfds[j].events = POLLOUT;
// 				break;
// 			}
// 		}
// 		continue;
// 	}
// }

bool handle_cgi_timeout(Client& client, std::vector<struct pollfd>& pfds, 
                       std::map<int, Client*>& cgi_pipes, int timeout) {
    if (!client.is_cgi_running()) {
        return false;
    }
	(void) cgi_pipes;
	(void) pfds;
	if (check_cgi_timeout(client, timeout)) {
        client.cgi_output = "HTTP/1.1 504 Gateway Timeout\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: 54\r\n"
                           "\r\n"
                           "<html><body><h1>504 Gateway Timeout</h1></body></html>";
				
        client.set_error_code(504);
        client.set_cgi_running(0);
        client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);
        client.set_cgi_start_time();

        return true;  // Timeout occurred
    }
    return false;  // No timeout
}