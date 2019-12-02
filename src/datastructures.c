#include "settings.h"
#include "filesystem.h"

#include "datastructures.h"

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

struct Inode getInode(int id) {
    struct Inode toReturn;
    readFS(INODE_OFFSET+id*sizeof(struct Inode), (void*)& toReturn, sizeof(struct Inode));
    return toReturn;
}

void createInode(int id, int parentId, int block, int type) {
    struct Inode newInode;
    newInode.id = id;
    newInode.size = 0;
    newInode.numOfBlocks = 1;
    newInode.direct[0] = block;
    newInode.direct[1] = -1;
    newInode.indirect = -1;

    if (type == 0) {
        newInode.numOfFiles = 0;
    } else {
        newInode.numOfFiles = 2;

        struct Mapping self = createMapping(".", id);
        writeFS(DATA_OFFSET + newInode.direct[0]*BLOCK_SIZE, (void*)& self, sizeof(struct Mapping));

        struct Mapping parent = createMapping("..", parentId);
        writeFS(DATA_OFFSET + newInode.direct[0]*BLOCK_SIZE + sizeof(struct Mapping), (void*)& parent, sizeof(struct Mapping));
    }

    writeFS(INODE_OFFSET+sizeof(struct Inode) * id, (void*)& newInode, sizeof(struct Inode));
    return;
}

struct Mapping createMapping (char name[MAX_LENGTH_FILE_NAME], int id) {
    struct Mapping toReturn;
    strcpy(toReturn.name, name);
    toReturn.id = id;
    return toReturn;
}

struct Superblock getSuperblock() {
    struct Superblock superblock;
    readFS(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));
    return superblock;
}