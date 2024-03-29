#include "commands.h"
#include "filesystem.h"
#include "settings.h"
#include "datastructures.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>


int do_open(int currentId, const char* pathname) {
    char* string = strdup(pathname);

    if (string != NULL) {
        char* token = strsep(&string, "/");
        while (token != NULL) {
            if (strcmp(token, "") != 0) {
                struct Inode currentDir = getInode(currentId);
                int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
                int flag = 1;

                for (int i = 0; i < currentDir.numOfFiles; i++) {
                    struct Mapping mapping;
                    readFS(offset + i*sizeof(struct Mapping),
                                       (void*)& mapping, sizeof(struct Mapping));

                    if (strcmp(token, mapping.name) == 0) {
                        currentId = mapping.id;
                        flag = 0;
                        break;
                    }
                }

                if (flag) {
                    return -1;
                }
            }
            token = strsep(&string, "/");
        }
        return currentId;
	}
    return -1;
}

int do_write(int id, int offset, void *buf, int count) {
    struct Inode inode = getInode(id);

    if (inode.id != id || inode.numOfFiles != 0) {
        printf("File is directory or the file does not exist. Exit with code -1.\n");
        return -1;
    }

    if (offset>=BLOCK_SIZE*inode.numOfBlocks) {
        printf("Offset is at or past the end of file, no bytes are written.\n");
        return 0;
    }

    int blk;
    for (int i = 0; i<MAX_DATA_BLOCK; i++) {
        if (offset < BLOCK_SIZE*(i+1)) {
            blk=i;
            if (blk > inode.numOfBlocks) {
                printf("Block index is > inode.numOfBlocks, no bytes are written.\n");
                return 0;
            }
            break;
        }
    }

    int fd = open (FS_NAME, O_RDWR, 660);
    int accumulateWrite = 0;
    int dirDataBlkIndex = 0;
    int blkCreateCount = 0;
    while(count > 0) {
        if(blk == 0) {
            if (inode.direct[0] == -1) {
                printf("Error: No direct block 1. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            } else {
                dirDataBlkIndex = inode.direct[0];
            }
        } else if(blk == 1) {
            if (inode.direct[1] == -1) {
                printf("Error: No direct block 2. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            } else {
                dirDataBlkIndex = inode.direct[1];
            }
        } else if(blk >= 2) {
            printf("%d\n", inode.direct[1]);
            if (inode.indirect == -1) {
                printf("Error: No indirect block. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            }
            else {
                lseek(fd, DATA_OFFSET + inode.indirect * BLOCK_SIZE + (blk)*sizeof(int), SEEK_SET);
                read(fd, (void*)& dirDataBlkIndex, sizeof(int));
            }
        }
        if (offset > 0) {
            offset = offset - (blk * BLOCK_SIZE);
        }

        int writeCount = 0;
        if (count < BLOCK_SIZE-offset) {
            writeCount = count;
            count-= writeCount;
        } else {
            writeCount = BLOCK_SIZE-offset;
            count -= writeCount;
        }

        lseek(fd, DATA_OFFSET + dirDataBlkIndex * BLOCK_SIZE + offset, SEEK_SET);
        write(fd, buf + accumulateWrite, writeCount);

        accumulateWrite += writeCount;

        if (count > 0) {
            offset = 0;
            blk++;

            blkCreateCount++;
            if (blk == 1 && getInode(id).direct[1] == -1) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.numOfBlocks = 2;
                inode.direct[1] = nextAvailableBlock; //update

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));

            } else if(blk == 2 && getInode(id).indirect == -1) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.indirect = nextAvailableBlock; //update

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));

                nextAvailableBlock = nextAvailableBlock+1;

                inode.numOfBlocks = inode.numOfBlocks+1;

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                lseek(fd, DATA_OFFSET + inode.indirect*BLOCK_SIZE + (inode.numOfBlocks)*sizeof(int), SEEK_SET);
                write(fd, &nextAvailableBlock, sizeof(int));

                superblock.nextAvailableBlock = nextAvailableBlock+14;

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));
            } else if(blk > 2) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.numOfBlocks = inode.numOfBlocks+1;

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                lseek(fd, DATA_OFFSET + inode.indirect*BLOCK_SIZE + (inode.numOfBlocks)*sizeof(int), SEEK_SET);
                write(fd, &nextAvailableBlock, sizeof(int));

                superblock.nextAvailableBlock = nextAvailableBlock+14;

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));
            }
        }
    }

    inode.size = inode.size+accumulateWrite;
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &inode, sizeof(struct Inode));

    close(fd);
    return accumulateWrite;
}

int createIndirectBlk(struct Inode inode) {
    struct Superblock superblock = getSuperblock();
    int nextAvailableBlock = superblock.nextAvailableBlock;

    if (nextAvailableBlock == MAX_DATA_BLOCK) {
        printf("Reached MAX_DATA_BLOCK! Abort.\n");
        return -1;
    }

    inode.indirect = nextAvailableBlock; //update

    int fd = open (FS_NAME, O_RDWR, 660);
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &inode, sizeof(struct Inode));

    superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

    lseek(fd, SB_OFFSET, SEEK_SET);
    write(fd, &superblock, sizeof(struct Superblock));

    close(fd);
    return nextAvailableBlock;
}

int do_read(int id, int offset, void* buf, int count) {
    struct Inode iNode = getInode(id);
    if (iNode.id != id || iNode.numOfFiles != 0) {
        return -1;
    }

    if (offset >= iNode.size) {
        return 0;
    }

    int block = (int) floor (offset / BLOCK_SIZE);

    int fd = open (FS_NAME, O_RDWR, 660);
    int readCount;
    int accumulateRead = 0;
    int dirDataBlkIndex = 0;

    while(count > 0) {
        if (block == 0) {
            if (iNode.direct[0] == -1) {
                printf("Error: No direct block 1. Return bytes read.\n");
                return accumulateRead;
            } else {
                dirDataBlkIndex = iNode.direct[0];
            }
        } else if (block == 1) {
            if (iNode.direct[1] == -1) {
                printf("Error: No direct block 2. Return bytes read.\n");
                return accumulateRead;
            } else {
                dirDataBlkIndex = iNode.direct[1];
            }
        } else if (block >= 2) {
            if (iNode.indirect == -1) {
                printf("Error: No indirect block. Return bytes read.\n");
                return accumulateRead;
            } else {
                lseek(fd, DATA_OFFSET + iNode.indirect * BLOCK_SIZE + (block)*sizeof(int), SEEK_SET);
                read(fd, (void *)& dirDataBlkIndex, sizeof(int));
            }
        }

        if (offset > 0) {
            offset = offset - (block * BLOCK_SIZE);
        }

        lseek(fd, DATA_OFFSET + dirDataBlkIndex * BLOCK_SIZE + offset, SEEK_SET);

        if (count < BLOCK_SIZE-offset) {
            readCount = count;
            count -= readCount;
        } else {
            readCount = BLOCK_SIZE-offset;
            count -= readCount;
        }

        read(fd, buf+accumulateRead, readCount);
        accumulateRead += readCount;
        if (count > 0) {
            offset = 0;
            block++;
        }
    }

    close(fd);
    return accumulateRead;
}

void help() {
    printf("HELP:\n");
    printf("touch [filename] - create empty file\n");
    printf("cat [filename] - print the file on the standart output\n");
    printf("wrtapp [filename] [string] - write string to the end of the file.\n");
    printf("ls - list directory contents\n");
    printf("cd [dirname] - change the working directory\n");
    printf("mkdir [dirname] - make directories\n");
    printf("help - show information");
}


void do_touch(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFS(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name, mapping.name) == 0) {
            printf("A file with the same name exists.");
            return;
        }
    }

    // Get next free inode and block
    struct Superblock superblock = getSuperblock();
    int nextAvailableInode = superblock.nextAvailableInode;
    int nextAvailableBlock = superblock.nextAvailableBlock;

    // Create inode for new directory
    createInode(nextAvailableInode, currentDirId, nextAvailableBlock, 0);

    // Add new mapping for parent directory
    struct Mapping mapping = createMapping(name, nextAvailableInode);
    offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    writeFS(offset + currentDir.numOfFiles * sizeof(struct Mapping),
                       (void*)& mapping, sizeof(struct Mapping));

    // Update number of sons of parent dir
    currentDir.numOfFiles = currentDir.numOfFiles+1;
    writeFS(INODE_OFFSET+currentDirId*sizeof(struct Inode),
                       (void*)& currentDir, sizeof(struct Inode));

    // Finally update superblock
    superblock.nextAvailableInode = nextAvailableInode + 1;
    superblock.nextAvailableBlock = nextAvailableBlock + 1;
    writeFS(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));
}

void do_cat(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    int id = do_open (currentDirId, name);
    if (id == -1) {
        printf("File not found. ");
        return;
    }
    struct Inode inode = getInode(id);
    char *buf = malloc(inode.size);
    int code = do_read(inode.id, 0, buf, inode.size);

    if (code == -1) {
        printf("File is directory or the file does not exist.");
        return;
    } else {
        printf("%s", buf);
        return;
    }
}

void do_wrt_app(int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT]) {
    int num = do_open(currentDirId, name);
    struct Inode inode = getInode(num);
    if (num == -1) {
        printf("File not found. ");
        return;
    }

    do_write(num, inode.size, text, strlen(text));
}


void do_ls(int currentDirId) {
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFS(offset + i*sizeof(struct Mapping),
                           (void*)& mapping, sizeof(struct Mapping));

        struct Inode tmp = getInode(mapping.id);
        if (tmp.numOfFiles == 0) {
            printf("\033[22;34m %s \033[0m", mapping.name);
        } else {
            printf("\033[22;34m %s/ \033[0m", mapping.name);
        }
    }
}

int do_cd (int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    if (strcmp(name, "/") == 0) {
        printf("Current directory is root");
        return 0;
    }

    if (name[0] == '/') {
        currentDirId = 0;
    }

    int temp = do_open(currentDirId, name);
    if (temp == -1) {
        printf("Can't find path. ");
        return currentDirId;
    } else {
        if (getInode(temp).numOfFiles == 0) {
            printf("This is not a directory.");
            return currentDirId;
        }
        printf("Current directory is %s", name);
        return temp;
    }
}

void do_mkdir(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFS(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name,mapping.name)==0) {
            printf("A directory with the same name has been found");
            return;
        }
    }

    // Get next free inode and block
    struct Superblock superblock = getSuperblock();
    int nextAvailableInode = superblock.nextAvailableInode;
    int nextAvailableBlock = superblock.nextAvailableBlock;

    // Create inode for new directory
    createInode(nextAvailableInode, currentDirId, nextAvailableBlock, 1);

    // Add new mapping for parent directory
    struct Mapping mapping = createMapping(name, nextAvailableInode);
    offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    writeFS (offset + currentDir.numOfFiles * sizeof(struct Mapping),
                       (void*)& mapping, sizeof(struct Mapping));

    // Update number of sons of parent dir
    currentDir.numOfFiles = currentDir.numOfFiles+1;
    writeFS(INODE_OFFSET+currentDirId*sizeof(struct Inode),
                       (void*)& currentDir, sizeof(struct Inode));

    // Finally update superblock
    superblock.nextAvailableInode = nextAvailableInode + 1;
    superblock.nextAvailableBlock = nextAvailableBlock + 1;
    writeFS(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));
}