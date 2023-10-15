#include <sstream>
#include <ws2tcpip.h>

#include "http_server.h"
#include "ns_plugin.h"

#define BUFFER_SIZE 128
#define HTTP_LF "\r\n"
#define BODY_SEP HTTP_LF HTTP_LF

HTTPRequest::HTTPRequest(SOCKET socket):
    socket(socket)
{
}

HTTPRequest::~HTTPRequest()
{
    this->close();
}

size_t HTTPRequest::content_length()
{
    auto& map = this->headers;
    header_map::const_iterator pos = map.find("Content-Length");
    if (pos == map.end())
        return 0;

    size_t result;
    std::istringstream sstream(pos->second);
    sstream >> result;

    return result;
}

void HTTPRequest::parse_headers(std::string raw)
{
    // @TODO validate first line for correct method and path
    std::istringstream iss(raw);

    for (std::string line; std::getline(iss, line, HTTP_LF[1]); )
    {
        if (line[line.size()-1] != HTTP_LF[0])
            break;

        line = line.substr(0, line.size()-1);
        if (line.empty())
            break;

        std::string::size_type sep_pos = line.find(":");

        if (line.size() <= sep_pos)
            continue;

        std::string key = line.substr(0, sep_pos);
        std::string value = line.substr(sep_pos+1);

        this->headers.try_emplace(key, value);
    }
}

void HTTPRequest::respond(std::string status_code, header_map response_headers, std::string response_body)
{
    if (this->socket == -1)
    {
        spdlog::error("Attempted to send response when socket is already closed");
        return;
    }

    std::ostringstream response;

    response << "HTTP/1.1 " << status_code << HTTP_LF;

    for (auto const& [key, val] : response_headers)
    {
        response << key << ": " << val << HTTP_LF;
    }

    response << HTTP_LF << response_body;

    std::string response_data = response.str(); 
    send(this->socket, response_data.c_str(), response_data.size(), 0);
}

void HTTPRequest::close()
{
    if (this->socket != -1)
    {
        closesocket(this->socket);
        this->socket = -1;
    }
}

HTTPServer::HTTPServer(unsigned long addr, unsigned short port)
{
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        spdlog::error("Failed to create socket");
        return;
    }

    struct sockaddr_in local = { 0 };
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = addr;
    local.sin_port = htons(port);

    if (bind(this->sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
    {
        spdlog::error("Failed to bindsocket ({})", WSAGetLastError());
        this->close();
        return;
    }

    if (listen(this->sock, 10) == SOCKET_ERROR)
    {
        spdlog::error("Failed to listen to socket");
        this->close();
        return;
    }

    spdlog::info("Initialized HTTPServer");
}

HTTPServer::~HTTPServer()
{
    this->close();
}

void HTTPServer::close()
{
    if (this->sock != -1)
    {
        closesocket(this->sock);
        this->sock = -1;
    }
}

HTTPRequest* HTTPServer::receive_request()
{
    if (this->sock == -1)
    {
        spdlog::error("Attempted to receive request without running web server");
        return nullptr;
    }

    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    spdlog::debug("awaiting HTTP request");

    SOCKET msg = accept(this->sock, (struct sockaddr*)&addr, &addr_len);
    if (msg == INVALID_SOCKET || msg == -1)
    {
        spdlog::error("Failed to receive packet ({})", WSAGetLastError());
        return nullptr;
    };

    spdlog::info("Connection opened by {}", inet_ntoa(addr.sin_addr));

    std::string content;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    bool parsed_header = false;
    std::string::size_type header_end;
    std::string::size_type expected_size;
    HTTPRequest* req = new HTTPRequest(msg);
    do
    {
        spdlog::debug("receiving buffer");
        int len = recv(msg, buffer, BUFFER_SIZE-1, 0);
        spdlog::debug("received buffer ({})", len);

        if (len == SOCKET_ERROR || len == 0)
            break;

        buffer[len] = '\0';

        content += buffer;

        if (!parsed_header && strstr(buffer, BODY_SEP))
        {
            parsed_header = true;
            req->parse_headers(content);

            header_end = content.find(BODY_SEP);
            expected_size = header_end + req->content_length() + strlen(BODY_SEP);
            spdlog::debug("Expecting size {}", expected_size);
        }

        if (parsed_header)
        {
            if (expected_size <= content.length())
            {
                req->set_body(content.substr(header_end+strlen(HTTP_LF)));
                break;
            }
        }
    }
    while(1);

    if (content.empty())
    {
        spdlog::error("Received no data ({})", WSAGetLastError());
        delete req;
        return nullptr;
    }

    spdlog::debug("Received Data ({})", content.length());

    return req;
}