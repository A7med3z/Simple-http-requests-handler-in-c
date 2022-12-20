#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <signal.h>
#include <pthread.h>
#include <errno.h>

void send_file (FILE *file, int sock) {
    char data[1024] = {0};
    
    while (fgets(data, 1024, file) != NULL)
    {
        if (send(sock, data, sizeof(data), 0) == -1)
        {
            printf("error in sending file\n");
            exit(1);
        }
        memset(data, 0, sizeof(data));
    }
}

void receive_file (int sock, FILE* file) {
    char data[1024];

    while (1)
    {
        if (recv (sock, data, 1024, 0) <= 0)
        {
            break;
        }
        printf("%s\n", data);
        fputs(data, file);
        printf("%s\n", data);
        memset(data, 0, sizeof(data));
    }
    fclose(file);
}

int main (int argc, char *argv[])
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        printf("error in creating the socket.\n");
        exit(1);
    }
    
    int option = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&option, sizeof(option));
    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(8080);
    
    if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) != 0)
    {
        printf("bind error\n");
        exit(1);
    }
    
    if (listen(serverSocket, 4) != 0) 
    {
        printf("listening error\n");
        exit(1);
    }
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    int clientAddressLength = 0;
    while (1)
    {
        memset(&clientAddress, 0, sizeof(clientAddress));
        int serverConnection = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (serverConnection == -1)
        {
            printf("failed to accept the request\n");
            exit(1);
        }
        fflush(stderr); 
        pid_t pid = fork();
        if (pid == -1)
        {
            close(serverConnection);
            continue;
        }
        if (pid == 0)
        {
            close(serverSocket);
            char buffer[1024];
            while (1)
            {
                memset(buffer, 0, sizeof(buffer));
                FILE* file = fopen("file_name.txt", "w");
                while (1)
                {
                    if (recv (serverConnection, buffer, 1024, 0) <= 0)
                    {
                    break;
                    }
                    printf("%s\n", buffer);
                    fputs(buffer, file);
                    printf("%s\n", buffer);
                    memset(buffer, 0, sizeof(buffer));
                }
                    fclose(file);
                if (buffer[0] == 'q')
                {
                    close(serverConnection);
                    exit(15);
                }
                send(serverConnection, buffer, strlen(buffer), 0);
            }
        }
        else
        {
            close(serverConnection);
        }
    }
    return 0;
}