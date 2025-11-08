# Webserv

HTTP server implementation in C++98 with CGI support.

---

## Table of Contents
- [Development Roadmap](#development-roadmap)
- [Architecture](#architecture)
- [Testing](#testing)
- [NGINX Reference](#nginx-reference)
- [Resources](#resources)

---

## Development Roadmap

### Connection
0. Parse Config file
1. Create a socket() - this will return an fd
2. Bind assigns an address
3. SOCK_STREAM will receive the connections
4. Listen for connections - use poll or similar and be able to handle all of them
5. Accept or reject

### Reading and Parse Request
6. Read the request
7. Parse the request

**Example Raw HTTP Request:**
```http
=== Raw HTTP Request from client 4 ===
GET / HTTP/1.1
Host: localhost:8080
User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate
=== End Request ===
```

8. Route to content_static or content_cgi (pipes or other solution)

### Response
9. Send response back (write) or start a fork for CGI

**Example NGINX Response:**
```http
HTTP/1.1 200 OK
Server: nginx/1.18.0
Date: Thu, 21 Aug 2025 16:56:33 GMT
Content-Type: text/html
Content-Length: 612
Last-Modified: Thu, 21 Aug 2025 16:56:21 GMT
Connection: keep-alive
ETag: "68a74fb5-264"
Accept-Ranges: bytes
```

### Handle Signals
10. Listen for signals and close connection on exit

---

## Architecture

### Key Structures

**sockaddr:**
```c
struct sockaddr {
    sa_family_t sa_family;
    char        sa_data[14];
};
```

**in_addr:**
```c
struct in_addr {
    uint32_t s_addr;  // 32-bit IPv4 address in network byte order
};
```

**pollfd:**
```c
int poll(struct pollfd fds[], nfds_t nfds, int timeout);

struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // on return, bitmap of events that occurred
};
```

### Parser Implementation
- **Config Parser**: Parses server configuration files
- **Request Parser**: Handles HTTP request parsing
- **Arena Implementation**: Memory management for headers and location blocks

---

## Testing

### Siege (Load Testing)

**Basic Commands:**
```bash
# Basic load test: 20 concurrent users, 5 repetitions each
siege -c 20 -r 5 http://localhost:8080

# Test multiple URLs: 3 concurrent users, 1 repetition each
siege -c 3 -r 1 -f urls.txt

# Benchmark mode: 50 connections for 1 minute
siege -c50 -t1M -b 'http://localhost:8080/cgi-bin/test.py'

# POST request with form data: 10 connections for 30 seconds
siege -c10 -t30S --content-type "application/x-www-form-urlencoded" \
  'http://localhost:8080/cgi-bin/post_foto.py POST name=cat&color=orange'

# Fixed total requests: 50 users × 10 requests = 500 total requests
siege -c 50 -r 10 'http://localhost:8080/cgi-bin/test.py'
```

**Siege Options:**
- `-c` = concurrent users
- `-r` = repetitions per user
- `-t` = time duration (e.g., `1M` = 1 minute, `30S` = 30 seconds)
- `-b` = benchmark mode (no delays between requests)
- `-f` = file containing URLs to test

---

### cURL (HTTP Testing)

**Basic Commands:**
```bash
# Simple GET request
curl http://localhost:8080

# Verbose mode with custom header
curl -v -H "Connection: close" http://localhost:8080

# Get only HTTP status code (silent mode)
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080

# DELETE in CGI (send filename in body)
curl -X DELETE http://localhost:8080/cgi-bin/delete_uploads.py \
  -H "Content-Type: text/plain" \
  -d "lala.jpeg" \
  -i

# GET with body
curl -v -X GET http://localhost:8080/ -d 'foo=bar'

# POST request
curl -v -X POST http://localhost:8080/ -d 'foo=bar'
```

**cURL Options:**
| Option | Description |
|--------|-------------|
| `-X` | Specify HTTP method (GET, POST, DELETE, etc.) |
| `-v` | Verbose mode (shows request/response headers) |
| `-H` | Add custom header (e.g., `"Content-Type: text/plain"`) |
| `-d` | Send data in request body (e.g., `-d "filename.jpg"`) |
| `-i` | Include response headers in output (shows HTTP status, Content-Type, etc.) |
| `-s` | Silent mode (hides progress bar and errors) |
| `-o /dev/null` | Discard response body |
| `-w "%{http_code}"` | Write out only the HTTP status code |

**DELETE Request Breakdown:**
- `-X DELETE` → Use DELETE method instead of default GET
- `-H "Content-Type: text/plain"` → Tell server we're sending plain text data
- `-d "lala.jpeg"` → Send filename in request body (CGI reads from stdin)
- `-i` → Show response status code (200 OK, 404 Not Found, etc.)

---

### Telnet (Manual HTTP Testing)

**Example Chunked POST:**
```bash
$ telnet localhost 8080
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
POST /cgi-bin/echo_post.py HTTP/1.1
Host: localhost
Transfer-Encoding: chunked
Content-Type: text/plain
<press Enter again for blank line>
5
Hello
6
 World
0
<press Enter again for final blank line>
```

---

### Test Scripts

**Transfer Encoding Test:**
```bash
./prepare_chunked_request.sh kitty.jpg
```

---

## Useful Commands

**Process Management:**
```bash
ps aux | grep webserv          # Find webserv processes
kill -9 <PID>                  # Kill process by PID
ps aux | grep defunct          # Find zombie processes
```

---

## NGINX Reference

### Basic Commands
```bash
sudo systemctl start nginx     # Start NGINX
sudo systemctl status nginx    # Check status
sudo nginx -t                  # Test configuration
sudo nginx -s stop             # Stop NGINX
sudo nginx -s reload           # Reload configuration
sudo nginx -s reopen           # Reopen log files
sudo nginx -s quit             # Graceful shutdown
```

### Process Management
```bash
ps -ax | grep nginx            # List NGINX processes
sudo pkill nginx               # Kill all NGINX processes
sudo lsof -i :80               # Check what's using port 80
```

### Start Fresh
```bash
sudo pkill nginx               # Kill existing NGINX
sudo nginx                     # Start NGINX
ps aux | grep nginx            # Verify it's running
sudo lsof -i :80               # Confirm port 80 is in use
sudo nginx -t                  # Validate configuration
```

### Graceful Shutdown
```bash
# If master process ID is 1628:
kill -s QUIT 1628
```

---

## Resources

### Documentation
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/split/index.html)
- [NGINX Configuration File Structure](https://nginx.org/en/docs/beginners_guide.html)
- [NGINX Worker Processes](https://nginx.org/en/docs/ngx_core_module.html#worker_processes)
## Structs

The sockaddr structure is defined as something like:
```c
struct sockaddr {
	sa_family_t sa_family;
	char        sa_data[14];
};
```

```c
in_addr struct:
struct in_addr {
    uint32_t s_addr;  // 32-bit IPv4 address in network byte order
};
```
```c
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
struct pollfd {
    int fd;         // the socket descriptor
    short events;   // bitmap of events we're interested in
    short revents;  // on return, bitmap of events that occurred
};
```
## Test Commands

### Siege (Load Testing)
```bash
# Basic load test: 20 concurrent users, 5 repetitions each
siege -c 20 -r 5 http://localhost:8080

# Test multiple URLs: 3 concurrent users, 1 repetition each
siege -c 3 -r 1 -f urls.txt

# Benchmark mode: 50 connections for 1 minute
siege -c50 -t1M -b 'http://localhost:8080/cgi-bin/test.py'

# POST request with form data: 10 connections for 30 seconds
siege -c10 -t30S --content-type "application/x-www-form-urlencoded" \
  'http://localhost:8080/cgi-bin/post_foto.py POST name=cat&color=orange'

# Fixed total requests: 50 users × 10 requests = 500 total requests
siege -c 50 -r 10 'http://localhost:8080/cgi-bin/test.py'
```

**Siege Options:**
- `-c` = concurrent users
- `-r` = repetitions per user
- `-t` = time duration (e.g., `1M` = 1 minute, `30S` = 30 seconds)
- `-b` = benchmark mode (no delays between requests)
- `-f` = file containing URLs to test

### cURL (HTTP Testing)
```bash
# Simple GET request
curl http://localhost:8080

# Simple UPLOAD request
curl -v -X POST http://localhost:8080/www/html/uploads/ -d "hello"

# Simple DELETE request
curl -X DELETE http://localhost:8080/uploads/upload_1.txt

# Verbose mode with custom header
curl -v -H "Connection: close" http://localhost:8080

# Get only HTTP status code (silent mode)
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080

# DELETE in CGI (send filename in body to manage_uploads.py)
curl -X DELETE http://localhost:8080/cgi-bin/delete_uploads.py \
  -H "Content-Type: text/plain" \
  -d "lala.jpeg" \
  -i
```

**cURL Options:**
- `-X` = specify HTTP method (GET, POST, DELETE, etc.)
- `-v` = verbose mode (shows request/response headers)
- `-H` = add custom header (e.g., `"Content-Type: text/plain"`)
- `-d` = send data in request body (e.g., `-d "filename.jpg"`)
- `-i` = include response headers in output (shows HTTP status, Content-Type, etc.)
- `-s` = silent mode (hides progress bar and errors)
- `-o /dev/null` = discard response body
- `-w "%{http_code}"` = write out only the HTTP status code

**Why these options for DELETE?**
- `-X DELETE` → Use DELETE method instead of default GET
- `-H "Content-Type: text/plain"` → Tell server we're sending plain text data
- `-d "lala.jpeg"` → Send filename in request body (CGI reads from stdin)
- `-i` → Show response status code (200 OK, 404 Not Found, etc.)

# Telnet
```
$ telnet localhost 8080
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
POST /cgi-bin/echo_post.py HTTP/1.1
Host: localhost
Transfer-Encoding: chunked
Content-Type: text/plain
<press Enter again for blank line>
5
Hello
6
 World
0
<press Enter again for final blank line>
```

# Test Transfer encoding
` ./prepare_chunked_request.sh kitty.jpg`


## Useful commands
- `ps aux | grep webserv`
- `kill -9 <PID>`
- `ps aux | grep defunct`

## NGINX commands
- `sudo systemctl start nginx`
- `sudo nginx -t`
- `sudo lsof -i :80` to check what is in :80
- `sudo systemctl status nginx`
- `sudo nginx -s stop`
- `sudo nginx -s reload`
- `sudo nginx -s reopen`
- `sudo nginx -s quit`
- `ps -ax | grep nginx`
- For example, if the master process ID is 1628, to send the QUIT signal resulting in nginx’s graceful shutdown, execute: `kill -s QUIT 1628`
- `sudo pkill nginx` and to verify it stopped `ps aux | grep nginx`

Now you simply need to start nginx fresh:
`sudo nginx`

Then check that it’s running:
`ps aux | grep nginx`
`sudo lsof -i :80`

And confirm your config is valid:
`sudo nginx -t`

To reload config file
`sudo nginx -s reload`

##### To test test1.config
1. GET 
Run:
```sh
curl -v -X GET http://localhost:8080/ -d 'foo=bar' 
``` 
-X GET → force GET
-d 'foo=bar' → add request body

Connection refuse response is:
```sh
 Trying ::1:8080...
* connect to ::1 port 8080 failed: Connection refused
*   Trying 127.0.0.1:8080...
* Connected to localhost (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.74.0
> Accept: */*
> Content-Length: 7
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 7 out of 7 bytes
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Server: nginx/1.18.0
< Date: Thu, 04 Sep 2025 18:26:48 GMT
< Content-Type: application/octet-stream
< Content-Length: 43
< Connection: keep-alive
< 
Request method: GET
Request body size: 146
* Connection #0 to host localhost left intact

```

Response ok:
```sh
*   Trying ::1:8080...
* connect to ::1 port 8080 failed: Connection refused
*   Trying 127.0.0.1:8080...
* Connected to localhost (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.74.0
> Accept: */*
> Content-Length: 7
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 7 out of 7 bytes
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Server: nginx/1.18.0
< Date: Thu, 04 Sep 2025 18:45:21 GMT
< Content-Type: application/octet-stream
< Content-Length: 43
< Connection: keep-alive
< 
Request method: GET
Request body size: 146
* Connection #0 to host localhost left intact

```

2. POST
Response for `curl -v -X POST http://localhost:8080/ -d 'foo=bar'` (-v verbose mode) 
```
Trying ::1:8080...
* connect to ::1 port 8080 failed: Connection refused
*   Trying 127.0.0.1:8080...
* Connected to localhost (127.0.0.1) port 8080 (#0)
> POST / HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.74.0
> Accept: */*
> Content-Length: 7
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 7 out of 7 bytes
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Server: nginx/1.18.0
< Date: Thu, 04 Sep 2025 19:09:20 GMT
< Content-Type: application/octet-stream
< Content-Length: 44
< Connection: keep-alive
< 
Request method: POST
Request body size: 147

```
This is supposed to infere is post because of -d
```sh
darotche@darotche:/etc/nginx/conf.d$ curl http://localhost:8080/ -d 'foo=bar'
Request method: POST
Request body size: 147
```

# TO READ
- https://beej.us/guide/bgnet/html/split/index.html
- NGINX Configuration File’s Structure https://nginx.org/en/docs/beginners_guide.html
- Worker processes https://nginx.org/en/docs/ngx_core_module.html#worker_processes
