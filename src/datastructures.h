#pragma once

#include "settings.h"


struct Inode {					/* The structure of inode, each file has only one inode */
    int id; 				    /* The inode number */
    int size; 				    /* The size of file */
    int numOfBlocks; 			/* The total numbers of data blocks */
    int direct[2]; 			    /* Two direct data block pointers */
    int indirect; 			    /* One indirect data block pointer */
    int numOfFiles; 			/* Amount of files in directory, 0 if file */
};

struct Mapping {			            /* Record file information in directory file */
    char name[MAX_LENGTH_FILE_NAME];    /* The file name in current directory */
    int id; 			                /* The corresponding inode number */
};


struct Superblock {				    /* The key information of filesystem */
    int inodeOffset; 			    /* The start offset of the inode region */
    int dataOffset; 			    /* The start offset of the data region */
    int maxInodeNum; 				/* The maximum number of inodes */
    int maxDataBlockNum; 			/* The maximum number of data blocks */
    int nextAvailableInode; 	    /* The index of the next free inode */
    int nextAvailableBlock; 	    /* The index of the next free block */
    int sizeOfBlock; 				/* The size per block */
};


struct Inode getInode(int id);

void createInode(int id, int parentId, int block, int type);

struct Mapping createMapping (char name[MAX_LENGTH_FILE_NAME], int id);


struct Superblock getSuperblock();


