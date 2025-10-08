# TO DO

### Connection
0. Parse Config file
1. create a socket(). this will return an fd
2. bind assigns an address
3. SOCK_STREAM will receive the connections
4. Listen for connections. Here we need to use poll or similar and be able to handle all of them.
5. Accept or reject. 

### Reading and parse request
6. Read the request. 
7. Parse the request
```json
	=== Raw HTTP Request from client 4 ===
	GET / HTTP/1.1
	Host: localhost:8080
	User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:142.0) Gecko/20100101 Firefox/142.0
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
	Accept-Language: en-US,en;q=0.5
	Accept-Encoding: gzip, defla
	=== End Request ===
```
8. Route to content_static or content_cgi. Pipes or other solution


### Response
9. Send response back(write) or start a for for cgi
This is NGINX Response:
```json
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
### Handle signals
10. Listen for signals? close connection on exit.



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


## Parser Config
## Parser Requests
Arena implementation.
- header
- location

## Test commands
- ` siege -c 20 -r 5 http://localhost:8080 ` (-c = concurrent users, -r = repetitions per user)
- ` siege -c 3 -r 1 -f urls.txt ` to run differents conections with differents urls
- `curl http://localhost:8080`
- `curl -v -H "Connection: close" http://localhost:8080` -H to write in Connection Header
- `siege -c50 -t1M -b 'http://localhost:8080/cgi-bin/test.py'` 50 connections 1 minute 

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