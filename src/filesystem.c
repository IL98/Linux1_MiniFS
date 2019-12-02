#include "filesystem.h"

#include "settings.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void readFS (int offset, void* data, int size) {
    int fd = open (FS_NAME, O_RDWR, 660);
    if (fd == -1) {
        printf("File '%s' doesn't exist. Try to create it manually.\n", FS_NAME);
        exit(0);
    }

    lseek(fd, offset, SEEK_SET);
    read(fd, data, size);
    close(fd);
}

void writeFS (int offset, const void* data, int size) {
    int fd = open (FS_NAME, O_RDWR, 660);
    if (fd == -1) {
        printf("File '%s' doesn't exist. Try to create it manually.\n", FS_NAME);
        exit(0);
    }

    lseek(fd, offset, SEEK_SET);
    write(fd, data, size);
    close(fd);
}