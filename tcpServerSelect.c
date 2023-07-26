#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include "tcpController.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main()
{
    int server_socket, client_sockets[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds, allfds;
    int max_fd, activity, i, valread, new_socket, sd;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6666);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Initialize client_sockets array
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    // Set the server socket in the allfds set
    FD_ZERO(&allfds);
    FD_SET(server_socket, &allfds);
    max_fd = server_socket;

    printf("Server listening on port 6666...\n");

    while (1)
    {
        // Copy allfds to readfds
        readfds = allfds;

        // Wait for activity on any of the sockets
        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL); // 第三个NULL表示阻塞

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select error");
        }

        // Check for incoming connection on the server socket
        if (FD_ISSET(server_socket, &readfds))
        {
            addrlen = sizeof(client_addr);
            if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            // Add new_socket to the client_sockets array
            for (i = 0; i < max_clients; i++)
            {
                if (client_sockets[i] == 0)
                {
                    client_sockets[i] = new_socket;
                    break;
                }
            }

            // Add new_socket to the allfds set
            FD_SET(new_socket, &allfds);

            // Update max_fd if necessary
            if (new_socket > max_fd)
            {
                max_fd = new_socket;
            }

            printf("New connection, socket fd is %d, IP is : %s, Port is : %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }

        // Check for data from clients
        for (i = 0; i < max_clients; i++)
        {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds))
            {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0)
                {
                    // Client disconnected
                    getpeername(sd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen);
                    printf("Host disconnected, IP is : %s, Port is : %d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    // Close the socket and remove from allfds set
                    close(sd);
                    FD_CLR(sd, &allfds);
                    client_sockets[i] = 0;
                }
                else
                {
                    // Echo back the message to the client
                    buffer[valread] = '\0';
                    // send(sd, buffer, strlen(buffer), 0);
                    printf("Received: %s\n", buffer);
                    jsonParse(buffer);
                }
            }
        }
    }

    // Close server socket
    close(server_socket);

    return 0;
}