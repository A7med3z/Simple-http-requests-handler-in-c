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

void send_file (char *name, int sock) {
    FILE* file = fopen (name, "rb");
    send(sock, name, sizeof(name), 0);

    int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    printf("size = %d\n", size);    
    send(sock, &size, sizeof(int), 0);
    
    char data[size];
    bzero(data, sizeof(data));
    
    while (!feof(file))
    {
        fread(data, 1, sizeof(data), file);
        send(sock, data, sizeof(data), 0);
        bzero(data, sizeof(data));
    }
    fclose(file);
    printf("finish sending\n");
}

void receive_file (int sock) {

    char name[1024];
    recv(sock, name, 1024, 0);
    printf("file name : %s\n", name);

    if (access(name, F_OK) == 0)
    {
        printf("file name already exists\n");
        exit(1);
    }
    
    int size;
    recv(sock, &size, sizeof(int), 0);
    printf("size = %d\n", size);
    
    char data[size];
    recv (sock, data, size, 0);
    FILE* file = fopen(name, "wb");
    fwrite(data, 1, sizeof(data), file);
    fclose(file);
    printf("finish receiving\n");
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

        pid_t pid = fork();

        if (pid == -1)
        {
            close(serverConnection);
            continue;
        }
        if (pid == 0)
        {
            close(serverSocket);
            receive_file(serverConnection);
        }
        else
        {
            close(serverConnection);
        }
    }
    return 0;
}