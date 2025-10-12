#include "CGI.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/wait.h>

CGI::~CGI() {
    if (_running && _pid > 0) {
        kill(_pid, SIGTERM);
        waitpid(_pid, NULL, WNOHANG);
    }
    
    if (_pipe_fd >= 0) {
        close(_pipe_fd);
    }
}

char** CGI::build_envp(const std::string& script_path, 
                      const std::string& method,
                      const std::string& http_version) {
    _env_storage.clear();
    _env_ptrs.clear();
    
    _env_storage.push_back("REQUEST_METHOD=" + method);
    _env_storage.push_back("SCRIPT_NAME=" + script_path.substr(script_path.find_last_of("/") + 1));
    _env_storage.push_back("SERVER_PROTOCOL=" + http_version);
    
    for (std::vector<std::string>::iterator it = _env_storage.begin();
         it != _env_storage.end(); ++it) {
        _env_ptrs.push_back(const_cast<char*>(it->c_str()));
    }
    _env_ptrs.push_back(NULL); // execve expects c strings
    return &_env_ptrs[0];
}

// bool CGI::run_cgi(const std::string& interpreter_path, const std::string& script_path,
//                  Client* client, std::vector<struct pollfd>& pfds, 
//                  std::map<int, Client*>& cgi_pipes) {
//     int pipefd[2];
//     if (pipe(pipefd) < 0) {
//         perror("pipe failed");
//         return false;
//     }
    
//     printf("=== CGI will run for Client %d ===================== \n\n", client->get_fd());
//     pid_t pid = fork();
    
//     if (pid < 0) {
//         perror("fork failed");
//         close(pipefd[0]);
//         close(pipefd[1]);
//         return false;
//     }
    
//     _start_time = std::time(NULL);
//     client->set_cgi_start_time();
    
//     if (pid == 0) {
//         // Child process
//         dup2(pipefd[1], STDOUT_FILENO);
//         close(pipefd[0]);
//         close(pipefd[1]);
        
//         client->set_cgi_running(1);
        
//         // Build environment variables
//         char** envp = client->build_envp(script_path);
        
//         // Use interpreter to execute script
//         char* const argv[] = {
//             (char*)interpreter_path.c_str(),
//             (char*)script_path.c_str(),
//             NULL
//         };
        
//         execve(interpreter_path.c_str(), argv, envp);
//         perror("execve failed");
//         _exit(1);
//     }
    
//     printf("=== CGI After execve ===================== \n\n");
//     printf("fd '%d' is being added to poll\n", pipefd[0]);
//     close(pipefd[1]);
//     pfds.push_back(Server::create_pollfd(pipefd[0], POLLIN, 0));
//     cgi_pipes[pipefd[0]] = client;
    
//     // Update internal state
//     _pipe_fd = pipefd[0];
//     _pid = pid;
//     _running = true;
    
//     client->set_cgi_pipe_fd(pipefd[0]);
//     client->set_cgi_pid(pid);
    
//     return true;
// }

// bool CGI::handle_cgi_read_from_pipe(std::vector<struct pollfd>& pfds, Client* client) {
//     char buf[1024];
//     ssize_t n;
    
//     printf("=== handle_cgi_read_from_pipe called for pipe fd %d =====================\n", _pipe_fd);
//     n = read(_pipe_fd, buf, sizeof(buf));
    
//     if (n > 0) {
//         printf("=== CGI Reading from pipe, got %zd bytes =====================\n", n);
//         buf[n] = '\0';
//         printf("=== CGI output chunk: %s =====================\n", buf);
//         _output.append(buf, n);
//         client->set_cgi_output(_output);
//         return false;  // Keep reading
//     }
    
//     if (n == 0)
//         return cgi_eof(pfds, client);
    
//     if (n < 0) {
//         printf("=== CGI read error: %s =====================\n", strerror(errno));
        
//         // Save PID before resetting
//         pid_t cgi_pid = _pid;
        
//         _running = false;
//         client->set_cgi_running(0);
//         client->set_cgi_pid(-1);
        
//         close(_pipe_fd);
//         waitpid(cgi_pid, NULL, WNOHANG);
//         return true;
//     }
    
//     return false;
// }

// bool CGI::cgi_eof(std::vector<struct pollfd>& pfds, Client* client) {
//     printf("=== CGI pipe closed (EOF), writing response =====================\n");
    
//     // Save PID before resetting
//     pid_t cgi_pid = _pid;
    
//     // Reset state
//     _running = false;
//     client->set_cgi_running(0);
//     client->set_cgi_pid(-1);
    
//     // Close pipe
//     close(_pipe_fd);
    
//     // Remove fd from pfds vector
//     for (size_t i = 0; i < pfds.size(); i++) {
//         if (pfds[i].fd == _pipe_fd) {
//             printf("Removing pipe fd %d from pfds\n", _pipe_fd);
//             pfds.erase(pfds.begin() + i);
//             break;
//         }
//     }
    
//     std::cout << "pipe fd " << client->get_cgi_pipe() << std::endl;
    
//     int ret_pid = waitpid(cgi_pid, NULL, WNOHANG);
//     printf("=== CGI complete output: pid %d, %s =====================\n", 
//            ret_pid, client->get_cgi_output().c_str());
    
//     return true;
// }

// bool CGI::check_cgi_timeout(int timeout_seconds) {
//     printf("\nCGI TIMEOUT CHECK NOW!\n");
//     if (!_running) {
//         return false;
//     }
    
//     time_t now = std::time(NULL);
//     time_t elapsed = now - _start_time;
    
//     printf("time elapsed: '%lld'\n", (long long)elapsed);
//     printf("cgi start time: '%lld'\n", (long long)_start_time);
//     printf("timeout: '%d'\n", timeout_seconds);
    
//     if (elapsed > timeout_seconds) {
//         std::cout << "XXXXXXXXXX CGI Timeout" << std::endl;
        
//         if (_pid > 0) {
//             kill(_pid, SIGTERM);
//             usleep(100000);
            
//             if (kill(_pid, 0) == 0) {
//                 kill(_pid, SIGKILL);
//             }
//             waitpid(_pid, NULL, WNOHANG);
//         }
        
//         return true;
//     }
    
//     return false;
// }

// // WARNING: There's a mismatch in declaration - std::vector<> should be std::vector<struct pollfd>
// bool CGI::handle_cgi_timeout(std::vector<struct pollfd>& pfds, 
//                             std::map<int, Client*>& cgi_pipes,
//                             Client* client) {
//     const int CGI_TIMEOUT = 30;
    
//     if (!_running) {
//         std::cout << "XXXXX No cgi running" << std::endl;
//         return false;
//     }
    
//     (void)cgi_pipes;
//     (void)pfds;
    
//     if (check_cgi_timeout(CGI_TIMEOUT)) {
//         _output = "HTTP/1.1 504 Gateway Timeout\r\n"
//                  "Content-Type: text/html\r\n"
//                  "Content-Length: 54\r\n"
//                  "\r\n"
//                  "<html><body><h1>504 Gateway Timeout</h1></body></html>";
        
//         client->set_cgi_output(_output);
//         client->set_error_code(504);
//         client->set_cgi_pid(-1);
//         client->set_cgi_start_time();
        
//         return true;
//     }
    
//     return false;
// }
