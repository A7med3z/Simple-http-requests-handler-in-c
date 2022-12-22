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

#define BLOCK_SIZE (50 * 1024)

void send_file (char *name, int sock) {
    FILE* file = fopen (name, "rb");
    send(sock, name, sizeof(name), 0);

    int size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    printf("size = %d\n", size);    
    send(sock, &size, sizeof(int), 0);
    
    char* data = (char*) malloc(sizeof(char) * size);
    fread(data, 1, size, file);

    int num_blocks = size / BLOCK_SIZE;
    printf("%d\n", size);
    printf("%d\n", BLOCK_SIZE);
    printf("%d\n", num_blocks);
    int lastBlockSize = size % BLOCK_SIZE;
    int offset = 0;

    while (num_blocks > 0)
    {
        send(sock, data + (offset * BLOCK_SIZE), BLOCK_SIZE, 0);
        offset++;
        num_blocks --;
    }

    send(sock, data + offset * BLOCK_SIZE, lastBlockSize, 0);
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
        return;
    }
    
    int size;
    recv(sock, &size, sizeof(int), 0);
    printf("size = %d\n", size);
    
    char* data = (char*) malloc(sizeof(char) * size);

    int total_recrived = 0;
    int remaining = size;
    int received_blobk;
    
    while (remaining > 0)
    {
        received_blobk = recv (sock, data + total_recrived, remaining, 0);
        total_recrived += received_blobk;
        remaining = size - total_recrived;
    }

    FILE* file = fopen(name, "wb");        
    fwrite(data, 1, size, file);
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
            // char name[100];
            // char type[20];
            // char request[100];
            // recv(serverConnection, request, 100, 0);
            // sscanf(request, "HTTP/1.1 %s %s", type, name);
            // printf("%s\n", request);
            // printf("%s\n", type);
            // printf("%s\n", name);
            
            // if (strcmp("POST", type) == 0)
            // {
                receive_file(serverConnection);
            // } else if (strcmp("GET", type) == 0)
            // {
            //     send_file(name, serverConnection);
            // } else {
            //     printf("bad request\n");
            //     exit(1);
            // }  
        }
        else
        {
            close(serverConnection);
        }
    }
    return 0;
}