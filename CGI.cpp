#include "CGI.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>


bool CGI::run(Client& client, std::vector<struct pollfd>& pfds) {
    int pipefd_out[2]; // To Server
    int pipefd_in[2]; // From Client
    pipe(pipefd_out);
    pipe(pipefd_in);

    //printf("=== CGI will run for Client %d ===================== \n\n", client.get_fd());
    pid_t pid = fork();

    this->set_start_time();
    this->set_running(true);
    if (pid == 0) {
        dup2(pipefd_out[1], STDOUT_FILENO);
        dup2(pipefd_in[0], STDIN_FILENO);
        close(pipefd_out[0]);
        close(pipefd_out[1]);
        close(pipefd_in[0]);
        close(pipefd_in[1]);

        std::string abs_script = this->get_script_filename();
        std::string docroot    = this->get_document_root();

        size_t slash = abs_script.find_last_of('/');
        std::string script_dir = ".";
        std::string script_base = abs_script;
        if (slash != std::string::npos) {
            script_dir = abs_script.substr(0, slash);
            script_base = abs_script.substr(slash + 1);
            if (chdir(script_dir.c_str()) != 0) {
                perror("chdir");
                _exit(127);
            }
        }

        char ** envp = this->build_envp(
            client.get_method(),
            client.get_http_version(),
            client.get_header("Content-Length"),
            client.get_header("Content-Type")
        );

        const std::string& interp = this->get_interpreter();
        char* const argv[] = {
            const_cast<char*>(interp.c_str()),
            const_cast<char*>(script_base.c_str()),
            NULL
        };

        execve(interp.c_str(), argv, envp);
        perror("execve");
        _exit(127);
    }
    //printf("=== CGI After execvefd '%d' is being added to poll\n \n\n", pipefd_out[0]);
    close(pipefd_out[1]);
    close(pipefd_in[0]);
    pfds.push_back(Server::create_pollfd(pipefd_out[0], POLLIN, 0));
    this->set_stdout(pipefd_out[0]);
    this->set_written(0);
    this->set_pid(pid);

    if (client.get_method() == "POST" && !client.get_body().empty()) {
    //    printf("=== POST request with %zu bytes body =====================\n", client.get_body().length());
        this->set_writing(true);
        this->set_stdin(pipefd_in[1]);
        pfds.push_back(Server::create_pollfd(pipefd_in[1], POLLOUT, 0));
    } else {
    //    printf("NO PIPE STDIN\n");
        close(pipefd_in[1]);
        this->set_stdin(-1);
        this->set_writing(false);
    }

    return true;
}

CGI::~CGI() {
    if (_running && _pid > 0) {
        kill(_pid, SIGTERM);
        waitpid(_pid, NULL, WNOHANG);
    }
    
    if (_stdout_fd >= 0) {
        close(_stdout_fd);
    }
}

char** CGI::build_envp(const std::string& method,
                      const std::string& http_version,
					  const std::string& content_length,
					  const std::string& content_type) {
    _env_storage.clear();
    _env_ptrs.clear();
    
    _env_storage.push_back("REQUEST_METHOD=" + method);
    _env_storage.push_back("SCRIPT_FILENAME=" + get_script_filename());   // full path
    _env_storage.push_back("SCRIPT_NAME=" + get_script_name());        // URI path
    _env_storage.push_back("DOCUMENT_ROOT=" + get_document_root());
    _env_storage.push_back("SERVER_PROTOCOL=" + http_version);
	_env_storage.push_back("CONTENT_LENGTH=" + content_length);
	_env_storage.push_back("CONTENT_TYPE=" + content_type);
    
    for (std::vector<std::string>::iterator it = _env_storage.begin();
         it != _env_storage.end(); ++it) {
        _env_ptrs.push_back(const_cast<char*>(it->c_str()));
    }
    _env_ptrs.push_back(NULL); // execve expects c strings
    return &_env_ptrs[0];
}

bool CGI::cgi_eof(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds)
{
//    printf("=== CGI pipe closed (EOF), writing response =====================\n");

    // Save PID before resetting it!
    pid_t cgi_pid = client.get_cgi_pid();
    client.set_cgi_running(0);
    client.set_cgi_pid(-1);

    // Close pipe and wait for child
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
    client.set_cgi_stdout_fd(-1);
    client.set_cgi_stdin_fd(-1);
    client.get_cgi().reset();
    return true;  // CGI finished
}

bool CGI::handle_cgi_read_from_pipe(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds) {
    char buf[1000];
    ssize_t n;
    n = read(pipe_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        client.cgi_output.append(buf, n);
        return false;  // Keep reading
    }
    if (n == 0) {
    //    printf("\033[33mDEBUG BEFORE EOF: client_fd=%d, cgi_stdin_fd=%d, cgi_written=%d, body_len=%zu, content-length='%s'\033[0m\n",
        client.get_fd(),
        client.get_cgi_stdin_fd(),
        client.get_cgi_written(),
        client.get_body().size(),
        client.get_header("Content-Length").c_str();
        return cgi_eof(pipe_fd, client, pfds);  // CGI finished
    }
    if (n < 0) {
    //    printf("\033[31m=== CGI read error: %s =====================\033[0m\n", strerror(errno));
        pid_t cgi_pid = client.get_cgi_pid();
        client.set_cgi_running(0);
        client.set_cgi_pid(-1);
        close(pipe_fd);
        waitpid(cgi_pid, NULL, WNOHANG);  // Use saved PID
        return true;
    }
    return false;
}

bool CGI::handle_cgi_write_to_pipe(int pipe_fd, Client &client, std::vector<struct pollfd>& pfds) {
//    printf("=== handle_cgi_write_to_pipe called for pipe fd %d =====================\n", pipe_fd);
    const std::string body = client.get_body();
    ssize_t body_len = static_cast<ssize_t>(body.length());
    ssize_t written = static_cast<ssize_t>(client.get_cgi_written());
    ssize_t remaining = (written < body_len) ? body_len - written : 0;

//    printf("DEBUG: Content-Length header? %s, body_len=%zd, written=%zd, remaining=%zd\n",
//           client.get_header("Content-Length").c_str(), body_len, written, remaining);

    if (remaining == 0) {
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

    const char *buf = body.c_str() + written;
    ssize_t n = write(pipe_fd, buf, static_cast<size_t>(remaining));

    if (n > 0) {
        written += n;
        client.set_cgi_written(static_cast<int>(written));
    //    printf("=== Wrote %zd bytes to CGI stdin (%zd/%zd total) =====================\n", n, written, body_len);
        if (written == body_len) {
        //    printf("\033[32m=== All POST body written (%zd/%zd), pipe %d =====================\033[0m\n", written, body_len, pipe_fd);
            for (size_t i = 0; i < pfds.size(); i++) {
                if (pfds[i].fd == pipe_fd) { pfds.erase(pfds.begin() + i); break; }
            }
            close(pipe_fd);
            client.set_cgi_stdin_fd(-1);
            client.set_cgi_written(0);
            client.set_cgi_writing(0);
            return true;
        }
        int pfd_idx = find_pfd(pipe_fd, pfds);
        if (pfd_idx != -1)
            pfds[pfd_idx].events |= POLLOUT;
        return false;
    }
    return false;
}

bool CGI::check_cgi_timeout(Client& client, int timeout) {
    if (!client.is_cgi_running()) {
        return false;  // Not running CGI
    }
    time_t now = time(NULL);
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
            if (kill(cgi_pid, 0) == 0) {
                kill(cgi_pid, SIGKILL);
            }
            waitpid(cgi_pid, NULL, WNOHANG);
        }
        return true;
    }
    return false;
}

bool CGI::handle_cgi_timeout(Client& client) {
    if (!client.is_cgi_running() || !client.is_cgi()) {
        //std::cout << "XXXXX No cgi running or client is not cgi" << std::endl;
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
