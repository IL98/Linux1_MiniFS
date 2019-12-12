#pragma once
#include "settings.h"
#include "datastructures.h"


int do_open(int currentId, const char* pathname) ;


int do_write(int id, int offset, void *buf, int count) ;


int createIndirectBlk(struct Inode inode) ;



int do_read(int id, int offset, void* buf, int count);

void help(int sock);


void do_ls (int currentDirId, int sock);


int do_cd (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);


void do_mkdir(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);


void do_touch(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);


void do_cat (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock);


void do_wrt_app(int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT], int sock);