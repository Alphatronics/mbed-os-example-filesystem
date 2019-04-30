/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include <stdio.h>
#include <errno.h>

#include "setup.h"
#include "benchmark.h"
#include "storage.h"

Benchmark benchmark;

const char* MYFILE = "/" MOUNTPOINT "/numbers.txt";

void setupFile(const char * path)
{
    // Open the numbers file
    bool hasFile = exists_file(path);
    if(hasFile) 
        return;

    // Create the numbers file if it doesn't exist
    printf("No file found, creating a new file... ");
    fflush(stdout);
    FILE *f = fopen(path, "w+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    int err=0;
    for (int i = 0; i < 10; i++) {
        printf("\rWriting numbers (%d/%d)... ", i, 10);
        fflush(stdout);
        err = fprintf(f, "    %d\n", i);
        if (err < 0) {
            printf("Fail :(\n");
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
    }
    printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);

    printf("Seeking file... ");
    fflush(stdout);
    err = fseek(f, 0, SEEK_SET);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }

    printf("\rClosing \"%s\"... ", path);
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }
}

// Entry point for the example
int main() {
    powerup();
    printf("--- Mbed OS filesystem example ---\n");

    selectDisk(DISKTYPE_SD);

    mount();

    int err=0;

    setupFile(MYFILE);

    printf("Opening \"%s\"... ", MYFILE);
    fflush(stdout);
    FILE *f = fopen(MYFILE, "r+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) 
        exit(-1);

    // Go through and increment the numbers
    for (int i = 0; i < 10; i++) {
        printf("\rIncrementing numbers (%d/%d)... ", i, 10);
        fflush(stdout);

        // Get current stream position
        long pos = ftell(f);

        // Parse out the number and increment
        int32_t number;
        fscanf(f, "%ld", &number);
        number += 1;

        // Seek to beginning of number
        fseek(f, pos, SEEK_SET);
    
        // Store number
        fprintf(f, "    %ld\n", number);

        // Flush between write and read on same file
        fflush(f);
    }
    printf("\rIncrementing numbers (%d/%d)... OK\n", 10, 10);

    // Close the file which also flushes any cached writes
    printf("Closing \"%s\"... ", MYFILE);
    fflush(stdout);
    err = fclose(f);
    printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
    if (err < 0) {
        error("error: %s (%d)\n", strerror(errno), -errno);
    }
    
    ls("/fs/");

    // Display the numbers file
    printf("Content of file \"%s\":\r\n ", MYFILE);
    cat(MYFILE);

    unmount();
        
    printf("Mbed OS filesystem example done!\n");
}

