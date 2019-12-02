#include "settings.h"
#include "filesystem.h"
#include "datastructures.h"


int init_root() {
    struct Inode root;
    root.id = 0;
    root.size = 0;
    root.numOfBlocks = 1;
    root.direct[0] = 0;
    root.direct[1] = -1;
    root.indirect = -1;
    root.numOfFiles = 1;
    writeFS(INODE_OFFSET, (void*) &root, sizeof(struct Inode));

    struct Mapping rootDir = createMapping(".", 0);
    writeFS(DATA_OFFSET, (void*) &rootDir, sizeof(struct Mapping));

    return 0;
}

int init_superblock() {
    struct Superblock superblock;
    superblock.inodeOffset = INODE_OFFSET;
    superblock.dataOffset = DATA_OFFSET;
    superblock.maxInodeNum = MAX_INODE;
    superblock.maxDataBlockNum = MAX_DATA_BLOCK;
    superblock.sizeOfBlock = BLOCK_SIZE;
    superblock.nextAvailableInode = 1;
    superblock.nextAvailableBlock = 1;
    writeFS(SB_OFFSET, (void*) &superblock, sizeof(struct Superblock));
    return 0;
}

int main(int argc, char *argv[]) {

    init_superblock();
    init_root();

    return 0;
}
