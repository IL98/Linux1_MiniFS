#pragma once

#define TRUE 1

#define MKDIR "mkdir"
#define LS "ls"
#define TOUCH "touch"
#define CAT "cat"
#define CD "cd"
#define WRT_APP "wrtapp"
#define HELP "help"
#define EXIT "exit"

#define FS_NAME "MiniFS"    /* Must be the same in Makefile */

#define SB_OFFSET 512           /* The offset of superblock region*/
#define INODE_OFFSET 4096       /* The offset of inode region */
#define DATA_OFFSET 10485760    /* The offset of data region */
#define MAX_INODE 100           /* The maximum number of inode */
#define MAX_DATA_BLOCK 25600    /* The maximum number of block */
#define BLOCK_SIZE 4096         /* The size per block */
#define MAX_NESTING_DIR 10      /* The nesting number of directory */
#define MAX_COMMAND_LENGTH 50   /* The maximum command length */

#define MAX_LENGTH_FILE_NAME 10 /* The maximum length of file name */
#define MAX_LENGTH_TEXT 100     /* The maximum amount of chars to write to file */

#define SERVER "127.0.0.1"
#define PORT 1998
#define MAX_MESSAGE_LENGTH 256