#include "./src/settings.h"
#include "./src/commands.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t max_length = MAX_COMMAND_LENGTH;
 int currentDirId = 0;  //ROOT

int run() {

    help();
    while (TRUE)  {
        char command[10];

        char* str = NULL;

        printf("\n$ ");
        int result = getline(&str, &max_length, stdin);

        if (result == -1) {
            printf("Input read error");
            return -1;
        } else if (str[0] == '\n') {
            command[0] = '\0';
        } else {
            str[result-1] = '\0';
            char* token = strtok(str, " ");
            strcpy(command, token);
        }

        if (strcmp(command, LS) == 0) {
            do_ls(currentDirId);
            continue;
        }

        if (strcmp(command, CD) == 0) {
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the directory.");
                continue;
            }

            currentDirId = do_cd(currentDirId, name);
            continue;
        }

        if(strcmp(command, MKDIR) == 0){
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the directory.");
                continue;
            }

            do_mkdir(currentDirId, name);
            continue;
        }

        if (strcmp(command, TOUCH) == 0) {
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the file.");
                continue;
            }

            do_touch(currentDirId, name);
            continue;
        }

        if(strcmp(command, CAT) == 0) {
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Specify name of the file.");
              continue;
          }

          do_cat(currentDirId, name);
          continue;
        }

        if(strcmp(command, WRT_APP) == 0) {
            char* token = strtok(NULL, " ");
            char name[MAX_LENGTH_FILE_NAME];

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the file.");
                continue;
            }

            char text[MAX_LENGTH_FILE_NAME];
            token = strtok(NULL, " ");
            if (token==NULL) {
                printf("Specify text.");
                continue;
            }
            strcpy(text, token);

            do_wrt_app(currentDirId, name, text);
            continue;
        }

        if(strcmp(command, HELP) == 0) {
            help();
            continue;
        }

        if(strcmp(command, EXIT) == 0) {
            printf("Exit MiniFS.");
            return 0;
        }

        fprintf(stderr, "Unknown command %s", command);
  }

}


int main() {

    printf("Hello,  you are in Mini File System:\n");

    run();
    return 0;
}