#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>

#define RPC_PORT 26503
#define MAX_CONNECTIONS 5

class Plugin;

struct thread_info
{
    HANDLE thread_handle;
    SOCKET socket_fd;
};

class jsonrpc_server {
    private:
        Plugin* parent;
        struct thread_info threads[MAX_CONNECTIONS] = {0};

    public:
        jsonrpc_server(Plugin* plugin);
        ~jsonrpc_server();
    
};

#endif