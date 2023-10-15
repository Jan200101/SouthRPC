#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <string>
#include <map>
#include <ws2tcpip.h>

#include "http_server.h"

#define HTTP_OK "200 OK"

typedef std::map<std::string, std::string> header_map;

class HTTPRequest {
    private:
        SOCKET socket;

        std::string request_type;
        std::string path;
        header_map headers;
        std::string body;


    public:
        HTTPRequest(SOCKET socket);
        ~HTTPRequest();

        void parse_headers(std::string raw);
        void set_body(std::string body) { this->body = body; }
        size_t content_length();

        std::string get_body() { return this->body; }

        void respond(std::string status_code, header_map response_headers, std::string response_body);
        void close();
};

class HTTPServer {
    private:
        SOCKET sock = -1;

        void parse_header();

    public:
        HTTPServer(unsigned long addr, unsigned short port);
        ~HTTPServer();

        void close();
        HTTPRequest* receive_request();
};

#endif
