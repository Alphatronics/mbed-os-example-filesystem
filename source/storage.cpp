#include "storage.h"

// Block devices
#include "DataFlashBlockDevice.h"
#include "SDBlockDevice.h"

// File systems
#include "LittleFileSystem.h"
#include "FATFileSystem.h"


// Physical block device, can be any device that supports the BlockDevice API
static DataFlashBlockDevice at45_bd(
        MBED_CONF_DATAFLASH_SPI_MOSI,
        MBED_CONF_DATAFLASH_SPI_MISO,
        MBED_CONF_DATAFLASH_SPI_CLK,
        MBED_CONF_DATAFLASH_SPI_CS);
static SDBlockDevice sd_card_bd(
        MBED_CONF_SD_SPI_MOSI,
        MBED_CONF_SD_SPI_MISO,
        MBED_CONF_SD_SPI_CLK,
        MBED_CONF_SD_SPI_CS);

BlockDevice *current_bd = NULL;

// File system declaration
FATFileSystem fs(MOUNTPOINT);


// trigger an erase (via button)
void erase() {
    printf("Initializing the block device... ");
    fflush(stdout);
    int err = current_bd->init();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Erasing the block device... ");
    fflush(stdout);
    err = current_bd->erase(0, current_bd->size());
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    printf("Deinitializing the block device... ");
    fflush(stdout);
    err = current_bd->deinit();
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
}

void selectDisk(DiskType disktype)
{
    const char* diskname = (disktype == DISKTYPE_SD ? "SDCARD" : "AT45");

    current_bd = (disktype == DISKTYPE_SD ? (BlockDevice*)&sd_card_bd : (BlockDevice*)&at45_bd);
    printf("Selected Disk: %s\n", diskname);

    printf("Init BlockDevice %s\n", diskname);
    fflush(stdout);
    int err = current_bd->init();
    if (err) {
        error("error: %s (%d)\n", strerror(-err), err);
    }

    diskinfo();
}

void diskinfo() {
    printf("dataflash size: %llu bytes\n", current_bd->size());
    printf("dataflash read size: %llu bytes\n", current_bd->get_read_size());
    printf("dataflash program size: %llu bytes\n", current_bd->get_program_size());
    printf("dataflash erase size: %llu bytes\n", current_bd->get_erase_size());
}

void mount()
{
    // Try to mount the filesystem
    printf("Mounting the filesystem... ");
    fflush(stdout);
    int err = fs.mount(current_bd);
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem
        // this should only happen on the first boot
        printf("No filesystem found, formatting... ");
        fflush(stdout);
        err = fs.reformat(current_bd);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }
}

void unmount()
{
    // Tidy up
    printf("Unmounting... ");
    fflush(stdout);
    int err = fs.unmount();
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(-err), err);
    }
}

bool exists_dir(const char *path) 
{
    DIR *d = opendir(path);
    if (!d) {
        if(errno == ENOENT)
            return false;
        else
            error("Failed opening directory, error: %s (%d)\n", strerror(errno), -errno);
    }
    int err = closedir(d);
    if (err < 0)
        error("Failed closing directory, error: %s (%d)\n", strerror(errno), -errno);
    return true;
}

bool exists_file(const char *path) 
{
    FILE *f = fopen(path, "r");
    if (!f) {
        if(errno == ENOENT)
            return false;
        else
            error("Failed opening file, error: %s (%d)\n", strerror(errno), -errno);
    }
    int err = fclose(f);
    if (err < 0)
        error("Failed closing file, error: %s (%d)\n", strerror(errno), -errno);
    return true;
}

void mkdir(const char *path) 
{
    if (exists_dir(path) == true) {
        return;
    }

	int err = mkdir(path, 0666);
	if (err < 0)
        error("Failed creating directory, error: %s (%d)\n", strerror(errno), -errno);
}

void touch(const char *path)
{
    if(exists_file(path)) 
        return;

    FILE *f = fopen(path, "w+");
    if (!f)
        error("Failed creating file, error: %s (%d)\n", strerror(errno), -errno);
    int err = fclose(f);
    if (err < 0)
        error("Failed closing file, error: %s (%d)\n", strerror(errno), -errno);
}

void ls(const char* directory)
{
    int total=0;

    fflush(stdout);
    DIR *d = opendir(directory);
    if (!d) {
        error("Failed opening directory, error: %s (%d)\n", strerror(errno), -errno);
    }

    //read all directory entries
    while (true) {
        struct dirent *e = readdir(d);
        if (!e) {
            break;
        }

        printf("    %s\n", e->d_name);
        total++;
    }
    printf("Total %d files in %s directory.\r\n", total,  directory);
    fflush(stdout);
    int err = closedir(d);
    if (err < 0) {
        error("Failed closing directory, error: %s (%d)\n", strerror(errno), -errno);
    }
}

void cat(const char *path) 
{
    if(!exists_file(path)) {
        printf("ERROR FILE NOT FOUND!");
        return;
    }

    FILE *f = fopen(path, "r");
    if (!f)
        error("Failed opening file, error: %s (%d)\n", strerror(errno), -errno);

    //print file content
    int c;
    while ((c = fgetc(f)) != EOF) {
        printf("%c", c);
    }
    fflush(stdout);   

    int err = fclose(f);
    if (err < 0)
        error("Failed closing file, error: %s (%d)\n", strerror(errno), -errno);
}

void rm(const char *path) 
{
    if (exists_file(path) == false) {
        return;
    }

	int err = remove(path);
	if (err < 0)
        error("Failed removing file, error: %s (%d)\n", strerror(errno), -errno);
}

void rmdir(const char *path) 
{
    if (exists_dir(path) == false) {
        return;
    }

    DIR *d = opendir(path);
    if (!d)
        error("Failed opening directory, error: %s (%d)\n", strerror(errno), -errno);

    int err=0;
    //read all directory entries
    while (true) {
        struct dirent *e = readdir(d);
        if (!e)
            break;
        if( (e->d_name[0] == '.' && e->d_name[1] == '\0') || 
            (e->d_name[0] == '.' && e->d_name[1] == '.' && e->d_name[2] == '\0') )
            continue;
        
        char filename[50];
        sprintf (filename, "/fs/tags/%s", e->d_name);
        printf("rm %s\n", filename);
        err = remove(filename);
	    if (err < 0) {
            error("Failed removing file [FILE=%s], error: %s (%d)\n", e->d_name, strerror(errno), -errno);
        }
    } 

    err = closedir(d);
    if (err < 0) {
        error("Failed closing directory, error: %s (%d)\n", strerror(errno), -errno);
    }
	err = remove(path);
	if (err < 0) {
        error("Failed removing directory, error: %s (%d)\n", strerror(errno), -errno);
    }
}


long filesize(const char *path)
{
    long filesize=0;
    FILE *f = fopen(path, "r");
    if (!f)
        error("Failed opening file, error: %s (%d)\n", strerror(errno), -errno);

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);

    int err = fclose(f);
    if (err < 0)
        error("Failed closing file, error: %s (%d)\n", strerror(errno), -errno);

    return filesize;
}