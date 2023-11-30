#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define BACKLOG_SIZE 128
#define BUFFER_SIZE 2048

int main()
{
    char buffer[BUFFER_SIZE];

    char response[] = "HTTP/1.0 200 OK\r\n"
    "Server: webserver-c\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html>hello, world</html>\r\n";

    int my_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(my_socket_fd == -1)
    {
        perror("webserver (socket)");
        return 1;
    }
    printf("Socket created successfully\n");

    // Host address
    struct sockaddr_in host_address;
    int host_addrlen = sizeof(host_address);
    host_address.sin_family = AF_INET;
    host_address.sin_port = htons(PORT);
    host_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Create client address
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    int bind_result = bind(my_socket_fd, (struct sockaddr*) &host_address, host_addrlen);
    if(bind_result == -1)
    {
        perror("webserver (bind)");
        return 1;
    }
    printf("Socket successfully bound to address!\n");

    int listen_result = listen(my_socket_fd, BACKLOG_SIZE);
    if(listen_result == -1)
    {
        perror("webserver (listen)");
        return 1;
    }
    printf("Listen was successful\n");

    for(;;)
    {
        // Accept incoming connections
        int new_socket_fd = accept(my_socket_fd, (struct sockaddr*)&host_address, (socklen_t*)&host_addrlen);
        if(new_socket_fd == -1)
        {
            perror("webserver (accept)");
            return 1;
        }
        printf("Connection accepted successfully\n");

        // Get client address
        int sockn = getsockname(new_socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0)
        {
            perror("webserver (getsockname)");
            continue;
        }

        // read from the socket
        ssize_t read_result =  read(new_socket_fd, &buffer[0], BUFFER_SIZE);
        if (read_result < 0)
        {
            perror("webserver (read)");
            continue;
        }

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, version, uri);

        // write
        ssize_t write_result = write(new_socket_fd, response, strlen(response));
        if(write_result < 0)
        {
            perror("webserver (write)");
            continue;
        }

        // Close the socket
        close(new_socket_fd);
    }

    return 0;
}
