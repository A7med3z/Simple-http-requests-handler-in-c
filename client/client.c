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
        printf("%s\n", data);
        memset(data, 0, sizeof(data));
    }
}

void receive_file (int sock, char* file_name) {
    FILE* file;
    char data[1024];
    file = fopen(file_name, "w");
    while (1)
    {
        int n = recv (sock, data, 1024, 0);
        if (n <= 0)
        {
            break;
        }
        fprintf(file, "%s", data);
        memset(data, 0, sizeof(data));
    }
    
}

int main (int argc, char *argv[]) {
    printf("%s\n", argv[0]);

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
    
    char buffer[1024];
    memset(buffer, 0, 1024);

    pid_t pid = fork();

    if (pid == 0)
    {
        // scanf("%s\n",buffer);
        FILE* file = fopen ("1.txt", "r");
        send_file(file, clientSocket);
    }
    else
    {
        receive_file(clientSocket, buffer);
    }
    return 0;
}