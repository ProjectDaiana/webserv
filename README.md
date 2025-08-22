#TO DO

## Connection
1. create a socket(). this will return an fd
2. bind assigns an address
3. SOCK_STREAM will receive the connections
The sockaddr structure is defined as something like:

           struct sockaddr {
               sa_family_t sa_family;
               char        sa_data[14];
           }

		   
4. Listen for requests. Here we need to use poll or similar and be able to handle all of them. Using arena, circular buffer or ? 
5. Accept or reject. 
6. Send Response. This is NGINX Response:
`	HTTP/1.1 200 OK
	Server: nginx/1.18.0
	Date: Thu, 21 Aug 2025 16:56:33 GMT
	Content-Type: text/html
	Content-Length: 612
	Last-Modified: Thu, 21 Aug 2025 16:56:21 GMT
	Connection: keep-alive
	ETag: "68a74fb5-264"
	Accept-Ranges: bytes
`

## NGINX commands
- `sudo systemctl start nginx`
- `sudo nginx -t`
- `sudo lsof -i :80`
- `sudo systemctl status nginx`
- `sudo nginx -s stop`
- `sudo nginx -s reload`
- `sudo nginx -s reopen`
- `sudo nginx -s quit`
- `ps -ax | grep nginx`
- For example, if the master process ID is 1628, to send the QUIT signal resulting in nginx’s graceful shutdown, execute: `kill -s QUIT 1628`
- `sudo pkill nginx` and to verify it stopped `ps aux | grep nginx`



## Reading
- https://beej.us/guide/bgnet/html/split/index.html
- NGINX Configuration File’s Structure https://nginx.org/en/docs/beginners_guide.html
- Worker processes https://nginx.org/en/docs/ngx_core_module.html#worker_processes