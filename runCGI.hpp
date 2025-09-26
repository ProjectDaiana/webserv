#pragma once
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <errno.h>

bool run_cgi(const std::string& script_path, Client& client, std::vector<struct pollfd>& pfds, std::map<int, Client*>& cgi_pipes) {
    int pipefd[2];
    pipe(pipefd);
	printf("=== CGI will run now ===================== \n\n");
    pid_t pid = fork();
    if (pid == 0) {
        // ---- child ----
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

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

// bool run_cgi(const std::string& script_path, Client& client, std::vector<struct pollfd>& pfds, std::map<int, Client*>& cgi_pipes) {
//     int pipefd[2];
//     if (pipe(pipefd) == -1) {
//         perror("pipe failed");
//         return false;
//     }
    
//     printf("=== CGI will run now: %s =====================\n", script_path.c_str());
//     printf("=== CGI pipe fds: read=%d, write=%d =====================\n", pipefd[0], pipefd[1]);
    
//     pid_t pid = fork();
//     if (pid == -1) {
//         perror("fork failed");
//         close(pipefd[0]);
//         close(pipefd[1]);
//         return false;
//     }
    
//     if (pid == 0) {
//         // ---- child ----
//         printf("=== CGI child process started, PID=%d =====================\n", getpid());
        
//         // Test if script exists and is readable
//         if (access(script_path.c_str(), R_OK | X_OK) == -1) {
//             perror("Script not accessible");
//             _exit(1);
//         }
        
//         dup2(pipefd[1], STDOUT_FILENO);
//         dup2(pipefd[1], STDERR_FILENO);  // Also redirect stderr for debugging
//         close(pipefd[0]);
//         close(pipefd[1]);

//         char* const envp[] = {
//             (char*)"REQUEST_METHOD=GET",
//             (char*)"SCRIPT_NAME=test.py",
//             (char*)"SERVER_PROTOCOL=HTTP/1.1",
//             (char*)"PATH=/usr/bin:/bin",  // Add PATH for script interpreters
//             NULL
//         };

//         char* const argv[] = {
//             (char*)"/usr/bin/python3",     // Use explicit python path
//             (char*)script_path.c_str(),
//             NULL
//         };

//         printf("=== CGI about to execve =====================\n");
//         execve("/usr/bin/python3", argv, envp);
        
//         // If execve fails, try without explicit python
//         char* const argv2[] = {
//             (char*)script_path.c_str(),
//             NULL
//         };
//         execve(script_path.c_str(), argv2, envp);
        
//         perror("execve failed");
//         _exit(1);
//     }
    
//     // ---- parent ----
//     printf("=== CGI parent process, child PID=%d =====================\n", pid);
//     close(pipefd[1]);  // Close write end in parent
    
//     // Add the pipe fd to poll with debug
//     printf("=== Adding CGI pipe fd %d to poll array =====================\n", pipefd[0]);
//     pfds.push_back(Server::create_pollfd(pipefd[0], POLLIN, 0));
    
//     cgi_pipes[pipefd[0]] = &client;
//     client.set_cgi_pipe_fd(pipefd[0]);
//     client.set_cgi_pid(pid);
//     client.set_cgi_running(1);

//     printf("=== CGI setup complete, pipe_fd=%d, pid=%d =====================\n", pipefd[0], pid);
    
//     // Print current pfds array for debugging
//     printf("=== Current pfds array size: %zu =====================\n", pfds.size());
//     for (size_t i = 0; i < pfds.size(); ++i) {
//         printf("    pfds[%zu]: fd=%d, events=%d\n", i, pfds[i].fd, pfds[i].events);
//     }
    
//     return true;
// }

bool handle_cgi_write(int pipe_fd, Client &client) {
    char buf[1000];
    ssize_t n;
    
    printf("=== handle_cgi_write called for pipe fd %d =====================\n", pipe_fd);
    n = read(pipe_fd, buf, sizeof(buf));
    
    if (n > 0) {
        printf("=== CGI Reading from pipe, got %zd bytes =====================\n", n);
        // Null terminate for debug output
        buf[n] = '\0';
        printf("=== CGI output chunk: %s =====================\n", buf);
        client.cgi_output.append(buf, n);
        return false;  // Keep reading
    }

    if (n == 0) {
        printf("=== CGI pipe closed (EOF), writing response =====================\n");    
		
        // Save PID before resetting it!
        pid_t cgi_pid = client.get_cgi_pid();
        
        // Reset client CGI state
        client.set_cgi_running(0);
        client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1); 
		
        // Close pipe and wait for child
        close(pipe_fd);
        int ret_pid = waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!
        printf("=== CGI complete output: pid %d,  %s =====================\n", ret_pid, client.cgi_output.c_str());

        return true;  // CGI finished
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

// bool handle_cgi_write(Client &client, int pipe_fd) {
//     char buf[1000];
//     ssize_t n;
    
//     std::cout << "handle cgi write" << std::endl;
//     // while ((n = read(pipe_fd, buf, sizeof(buf))) > 0) {
// 	// 	client.cgi_output.append(buf, n);
//     // }
    
// 	n = read(pipe_fd, buf, sizeof(buf));
// 	if (n > 0) {
//         printf("=== CGI Reading from pipe ===================== \n\n");
//         client.cgi_output.append(buf, n);
//         return false;  // Keep reading
//     }

//     if (n == 0) {
//         printf("=== CGI writing response ===================== \n\n");    
//         std::cout << client.cgi_output << std::endl;

//         // Save PID before resetting it!
//         pid_t cgi_pid = client.get_cgi_pid();
        
//         // Reset client CGI state
//         client.set_cgi_running(0);
//         client.set_cgi_pipe_fd(-1);
//         client.set_cgi_pid(-1); 

//         // Close pipe and wait for child
//         close(pipe_fd);
//         waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!

//         return true;  // CGI finished
//     }
    
//     if (n < 0) {
//         // Error occurred
//         printf("=== CGI read error ===================== \n\n");
        
//         // Save PID before resetting
//         pid_t cgi_pid = client.get_cgi_pid();
        
//         client.set_cgi_running(0);
//         client.set_cgi_pipe_fd(-1);
//         client.set_cgi_pid(-1);
        
//         close(pipe_fd);
//         waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID
//         return true;
//     }
	
// 	if (n < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
//         pid_t cgi_pid = client.get_cgi_pid();
        
//         client.set_cgi_running(0);
//         client.set_cgi_pipe_fd(-1);
//         client.set_cgi_pid(-1);
        
//         close(pipe_fd);
//         waitpid(cgi_pid, NULL, WNOHANG); 
//         return true;
//     }
    
//     return false;
// }

// std::string run_cgi(const std::string& script_path, int client_fd) {
//     int pipefd[2];
//     pipe(pipefd);
// 	printf("=== CGI will run now ===================== \n\n");
//     pid_t pid = fork();
//     if (pid == 0) {
//         // ---- child ----
//         dup2(pipefd[1], STDOUT_FILENO);
//         close(pipefd[0]);
//         close(pipefd[1]);

//         char* const envp[] = {
//             (char*)"REQUEST_METHOD=GET",
//             (char*)"SCRIPT_NAME=test.py",
//             (char*)"SERVER_PROTOCOL=HTTP/1.1",
//             NULL
//         };

//         char* const argv[] = {
//             (char*)script_path.c_str(),
//             NULL
//         };
// 		//write(pipefd[1], "hola\n", 5);
// 		//write(pipefd[0], "hola\n", 5);
//         execve(script_path.c_str(), argv, envp);
//         _exit(1); // only runs if execve fails
//     }

//     // ---- parent ----
//     close(pipefd[1]);
//     std::string output;
//     char buf[1024];
//     ssize_t n;
//     while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
//         output.append(buf, n);
// 	}
//     close(pipefd[0]);

//     waitpid(pid, NULL, 0);

// 	char buffer[32];
//     sprintf(buffer, "%lu", (unsigned long)output.size());
//     std::string content_length(buffer);
// 	//std::cout << "this is buffer: " << buffer << std::endl;
// 	//std::cout << "this is output: " << output << std::endl;
    
//     std::string response = "HTTP/1.1 200 OK\r\n";
//     //response += "Content-Length: " + content_length + "\r\n";
//     //response += "\r\n";
//     response += output;
//     write(client_fd, response.c_str(), response.size());
//     return output;
// }
