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


#include <iostream>
#include <string>

Benchmark benchmark;

#include<sstream>
template <typename T>
std::string to_string(T value)
{
    //create an output string stream
    std::ostringstream os ;

    //throw the value into the string stream
    os << value ;

    //convert the string stream into a string and return
    return os.str() ;
}

void testFile(const char * path, char* buffer, uint32_t size)
{
    FILE *f = fopen(path, "w+");
    if (!f) {
        error("Failed opening file, error: %s (%d)\n", strerror(errno), -errno);
    }

    size_t modified = fwrite (buffer , sizeof(char), size, f);
    if(modified != size)
        error("Failed writing data, error: %s (%d)\n", strerror(errno), -errno);
    
    int err = fclose(f);
    if (err < 0) {
        error("Failed closing file, error: %s (%d)\n", strerror(errno), -errno);
    }
}


void runAllTests()
{
    const uint8_t step = 32;
    uint32_t datasize = 32;
    const static char* MYFILE = "/" MOUNTPOINT "/rnd.bin";
    const static char TESTDATA[step+1] = { "my32charlongtestdatastringdjizle" };

    while(true) {
        //setup file
        rm(MYFILE);
        touch(MYFILE);
        //printf("Content of file \"%s\":\r\n ", MYFILE);
        //cat(MYFILE);

        //setup testbuffer with random data
        char * buffer = new char[datasize+1];
        buffer[datasize] = '\0';
        
        for(uint32_t i=0; i<datasize; i+=step) {
            memcpy(buffer, TESTDATA + i, step);
        }

        std::string name = std::string("Test: ") + to_string(datasize);

        benchmark.start(name);
        testFile(MYFILE, buffer, datasize);
        benchmark.stop();

        delete[] buffer;

        //printf("Content of file \"%s\":\r\n ", MYFILE);
        //cat(MYFILE);

        datasize += step;
        if(datasize>4096)
            break;
    }
}

// Entry point for the example
int main() {
    powerup();
    printf("---------------------------------------------------\n");
    printf("---------- Mbed OS filesystem benchmark -----------\n");
    printf("---------------------------------------------------\n");

    selectDisk(DISKTYPE_SD);

    mount();

    runAllTests();

    unmount();
        
    printf("Mbed OS filesystem benchmark done!\n");
}

