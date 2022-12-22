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
    printf("re = %d\n", (int)sizeof(name));
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
    free(data);
    printf("finish sending\n");
}

void receive_file (int sock,char*name) {

    printf("file name : %s\n", name);

    if (access(name, F_OK) == 0)
    {
        printf("file name already exists\n");
        exit(1);
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

int main () {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        printf("error in creating the socket\n");
    }
    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(8080);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        printf("connection error\n");
        exit(1);
    }

    char name[100];
    char request[100];
    char type[20];
    char host_name[20];
    int port_number;

    scanf("client_%s %s %s (%d)", type, name, host_name, &port_number);
    

    if (strcmp("get", type) == 0)
    {
        sprintf(request, "HTTP/1.1 GET %s %s %d", name, host_name, port_number);
        send(clientSocket, request, 100, 0);
        receive_file(clientSocket,name);
    } else if (strcmp("post", type) == 0)
    {
        sprintf(request, "HTTP/1.1 POST %s %s %d", name, host_name, port_number);
        send(clientSocket, request, 100, 0);
        send_file(name, clientSocket);
    } else {
        printf("bad request\n");
        exit(1);
    }    
    return 0;
}