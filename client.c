#include "./src/settings.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "stdlib.h"

static size_t max_message_length = MAX_MESSAGE_LENGTH;

int run() {
    while(1) {
        int sock = 0;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, SERVER, &server_address.sin_addr) <= 0) {
            printf("Invalid address/ Address not supported \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
            printf("Connection Failed \n");
            return -1;
        }

        char* command = NULL;
        char  str[20];

        char buffer[MAX_MESSAGE_LENGTH] = {0};

        printf("\n$ ");
        int result = getline(&command, &max_message_length, stdin);
        if (result == -1) {
            printf("getline() error. Try again.");
            return -1;
        }

        send(sock, command, strlen(command), 0);
        int valueRead = read(sock, buffer, 1024);

        if (command[0] == '\n') {
             str[0] = '\0';
        } else {
            int len = strlen(command);
            command[len-1] = '\0';
            char* token = strtok(command, " ");
            strcpy(str, token);
        }

        if(strcmp(str, EXIT) == 0) {
            printf("Exit MiniFS.");
            close(sock);
            exit(0);
            return 0;
        }

        printf(buffer);
        close(sock);


    }
    return 0;
}


int main(int argc, char const *argv[]) {
    printf("Hello,  you are in Mini File System:\n");

    int result = run();
    return result;
}