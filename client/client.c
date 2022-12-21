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

    if (access(name, F_OK) != 0)
    {
        printf("file doesn't exist\n");
        exit(1);
    }

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
    FILE* file = fopen(name, "w");
    fwrite(data, 1, sizeof(data), file);
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
    printf("enter file name: \n");
    gets(name);
    printf("-%s-\n", name);
    send_file(name, clientSocket);
    
    return 0;
}