#Client
-look for the ip address thru DNS
- establishes the TCP connection with the Server

#Server
-on server creation
-socket is created (for listening), then binded to get the socket's address
-then wait to receive the connection from the client

What to do


webserv.hpp         -> Main server orchestrator
├── Server.hpp      -> Individual server config (one per port/host)
├── Client.hpp      -> Represents a connected client
├── Config.hpp      -> Configuration parser
├── Request.hpp     -> HTTP request parser
├── Response.hpp    -> HTTP response builder
└── CGI.hpp         -> CGI handler
```

## ✅ Correct Flow Chart:
```
1. STARTUP PHASE
   ├─ Read & parse config file
   ├─ Create server socket(s) for each listen port
   ├─ bind() and listen() on each port
   ├─ Set all sockets to non-blocking
   └─ Create epoll instance, add all server sockets

2. MAIN LOOP (epoll_wait)
   ├─ epoll_wait() returns ready file descriptors
   │
   ├─ If ready fd is a SERVER socket (listening):
   │  ├─ accept() new connection → get client_fd
   │  ├─ Set client_fd to non-blocking
   │  ├─ Create Client object for this connection
   │  └─ Add client_fd to epoll (EPOLLIN | EPOLLOUT)
   │
   ├─ If ready fd is a CLIENT socket (EPOLLIN):
   │  ├─ recv() HTTP request data
   │  ├─ Parse HTTP request (method, path, headers, body)
   │  └─ Prepare response
   │
   ├─ If ready fd is a CLIENT socket (EPOLLOUT):
   │  ├─ send() HTTP response data
   │  └─ Close connection if response complete
   │
   └─ If request is CGI:
      ├─ fork() + execve() to run script
      ├─ Capture CGI output
      └─ Send output to client
