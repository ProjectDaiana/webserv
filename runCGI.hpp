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
    printf("fd '%d' is being added to poll\n", pipefd[0]);
	pfds.push_back(Server::create_pollfd(pipefd[0], POLLIN, 0)); //POLLIN
    cgi_pipes[pipefd[0]] = &client;
	client.set_cgi_pipe_fd(pipefd[0]);
    client.set_cgi_pid(pid);
	client.set_cgi_running(1);

	return true; 
}

bool cgi_eof(int pipe_fd, Client &client)
{
	printf("=== CGI pipe closed (EOF), writing response =====================\n");

        // Save PID before resetting it!
        pid_t cgi_pid = client.get_cgi_pid();
        // Reset client CGI state
        client.set_cgi_running(0);
        client.set_cgi_pipe_fd(-1);
        client.set_cgi_pid(-1);

        // Close pipe and wait for child
        close(pipe_fd);
        std::cout << "pipde fd "<< client.get_cgi_pipe() << std::endl;

        int ret_pid = waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID, not -1!
        printf("=== CGI complete output: pid %d,  %s =====================\n", ret_pid, client.cgi_output.c_str());
        return true;  // CGI finished

}

bool handle_cgi_write(int pipe_fd, Client &client) {
    char buf[10];
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

    if (n == 0) 
        return cgi_eof(pipe_fd, client);  // CGI finished
    
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
