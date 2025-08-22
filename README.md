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
4. Listen for requests
5. Accept or reject
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


## Reading
https://beej.us/guide/bgnet/html/split/index.html