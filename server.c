#include "./src/settings.h"
#include "./src/commands.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

static size_t max_length = MAX_COMMAND_LENGTH;

static size_t max_message_length = MAX_MESSAGE_LENGTH;

int run_shell(char str[MAX_MESSAGE_LENGTH], int currentDirId, int sock) {
    char command[10];

    if (str[0] == '\n') {
        command[0] = '\0';
    } else {
        int len = strlen(str);
        str[len-1] = '\0';
        char* token = strtok(str, " ");
        strcpy(command, token);
    }

    if (strcmp(command, LS) == 0) {
        do_ls(currentDirId, sock);
        return currentDirId;
    }

    if (strcmp(command, CD) == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            char* result = "Specify name of the directory.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }

        return do_cd(currentDirId, name, sock);
    }

    if(strcmp(command,  MKDIR) == 0){
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            char* result = "Specify name of the directory.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }

        do_mkdir(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, TOUCH) == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            char* result = "Specify name of the file.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }

        do_touch(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, CAT) == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            char* result = "Specify name of the file.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }

        do_cat(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, WRT_APP) == 0) {
        char* token = strtok(NULL, " ");
        char name[MAX_LENGTH_FILE_NAME];

        strcpy(name, token);
        if (token == NULL) {
            char* result = "Specify name of the file.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }

        char text[MAX_LENGTH_FILE_NAME];
        token = strtok(NULL, " ");
        if (token==NULL) {
            char* result = "Specify text.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }
        strcpy(text, token);

        do_wrt_app(currentDirId, name, text, sock);
        return currentDirId;
    }

    if (strcmp(command, HELP) == 0) {
        help(sock);
        return currentDirId;
    }

    if (strcmp(command, EXIT) == 0) {
        close(sock);
        return currentDirId;
    }
    char* tmp = "Unknown command. ";
    int len = 	strlen(tmp) + strlen(command) + 1;
    char *result = malloc(len);
    strcpy(result, tmp);
    strcat(result, command);
    send(sock, result, len, 0);
    return currentDirId;
}

int main(int argc, char const *argv[]) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        printf("error");
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        printf("error");
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0) {
        printf("error");
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        printf("error");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int address_len = sizeof(address);
    int currentDirId = 0; //root

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &address_len);
        if (new_socket < 0) {
            printf("error");
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_MESSAGE_LENGTH] = {0};
        int command = read(new_socket, buffer, 1024);
        printf("Message received: %s", buffer);

        currentDirId = run_shell(buffer, currentDirId, new_socket);
        close(new_socket);
    }
    return 0;
}