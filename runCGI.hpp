#pragma once
#include <unistd.h>
#include <sys/wait.h>
#include <string>

std::string run_cgi(const std::string& script_path, int client_fd) {
    int pipefd[2];
    pipe(pipefd);
	printf("=== CGI will run now ===================== \n\n");
    pid_t pid = fork();
    if (pid == 0) {
        // ---- child ----
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char* const envp[] = {
            (char*)"REQUEST_METHOD=GET",
            (char*)"SCRIPT_NAME=test.py",
            (char*)"SERVER_PROTOCOL=HTTP/1.1",
            NULL
        };

        char* const argv[] = {
            (char*)script_path.c_str(),
            NULL
        };
		//write(pipefd[1], "hola\n", 5);
		//write(pipefd[0], "hola\n", 5);
        execve(script_path.c_str(), argv, envp);
        _exit(1); // only runs if execve fails
    }

    // ---- parent ----
    close(pipefd[1]);
    std::string output;
    char buf[1024];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        output.append(buf, n);
	}
    close(pipefd[0]);

    waitpid(pid, NULL, 0);

	char buffer[32];
    sprintf(buffer, "%lu", (unsigned long)output.size());
    std::string content_length(buffer);
	//std::cout << "this is buffer: " << buffer << std::endl;
	//std::cout << "this is output: " << output << std::endl;

    
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += output;
    write(client_fd, response.c_str(), response.size());
    return output;
}
