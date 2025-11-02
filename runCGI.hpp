// #pragma once
// #include <unistd.h>
// #include <sys/wait.h>
// #include <string>
// #include <errno.h>
// #include <signal.h>
// #include "webserv.hpp"

// bool handle_cgi_write_to_pipe(int pipe_fd, Client &client,  std::vector<struct pollfd>& pfds) {
//     printf("=== handle_cgi_write_to_pipe called for pipe fd %d =====================\n", pipe_fd);
//     const std::string body = client.get_body();
//     ssize_t body_len = static_cast<ssize_t>(body.length());
//     ssize_t written = static_cast<ssize_t>(client.get_cgi_written());
//     ssize_t remaining = (written < body_len) ? body_len - written : 0;

//     printf("DEBUG: Content-Length header? %s, body_len=%zd, written=%zd, remaining=%zd\n",
//            client.get_header("Content-Length").c_str(), body_len, written, remaining);

//     if (remaining == 0) {
//         // nothing to write — cleanup this pipe
//         printf("=== CGI pipe in closed (done writing), cleaning up fd %d =====================\n", pipe_fd);
//         for (size_t i = 0; i < pfds.size(); i++) {
//             if (pfds[i].fd == pipe_fd) { pfds.erase(pfds.begin() + i); break; }
//         }
//         close(pipe_fd);
//         client.set_cgi_writing(0);
//         client.set_cgi_written(0);
//         client.set_cgi_stdin_fd(-1);
//         return true;
//     }

//     // write from the actual body buffer (single non-blocking write attempt)
//     const char *buf = body.c_str() + written;
//     ssize_t n = write(pipe_fd, buf, static_cast<size_t>(remaining));

//     if (n > 0) {
//         written += n;
//         client.set_cgi_written(static_cast<int>(written));
//         printf("=== Wrote %zd bytes to CGI stdin (%zd/%zd total) =====================\n", n, written, body_len);

//         if (written == body_len) {
//             // all bytes written -> cleanup
// 			printf("\033[32m=== All POST body written (%zd/%zd), pipe %d =====================\033[0m\n", written, body_len, pipe_fd);
//             for (size_t i = 0; i < pfds.size(); i++) {
//                 if (pfds[i].fd == pipe_fd) { pfds.erase(pfds.begin() + i); break; }
//             }
//             close(pipe_fd);
//             client.set_cgi_stdin_fd(-1);
//             client.set_cgi_written(0);
//             client.set_cgi_writing(0);
//             return true;
//         }
//         // partial write, keep fd in poll for POLLOUT and return to be retried on next event
//         int pfd_idx = find_pfd(pipe_fd, pfds);
//         if (pfd_idx != -1)
// 			pfds[pfd_idx].events |= POLLOUT;
//         return false;
//     }

//     return false;
// }