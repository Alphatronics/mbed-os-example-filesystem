#ifndef STORAGE_H
#define STORAGE_H

#include "BlockDevice.h"

#define MOUNTPOINT  "fs"

enum DiskType 
{
    DISKTYPE_SD,
    DISKTYPE_DATAFLASH
};

void erase();
void selectDisk(DiskType disktype);
void diskinfo();
void mount();
void unmount();

bool exists_dir(const char *path) ;
bool exists_file(const char *path);
void mkdir(const char *path);
void touch(const char *path);
void ls(const char* directory);
void cat(const char *path);
void rm(const char *path);
void rmdir(const char *path);
long filesize(const char *path);

#endif //STORAGE_H