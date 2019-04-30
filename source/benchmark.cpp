#include "benchmark.h"

Benchmark::Benchmark():
    timer(),
    name()
{
}

void Benchmark::start(std::string& name)
{
    this->name = name;
    timer.start();
}


void Benchmark::stop()
{
    timer.stop();
    int millis = timer.read_ms();
    int seconds = 0;
    if(millis >= 1000) {
        seconds = millis/1000;
        millis = millis%1000;
    }
    printf("BENCHMARK %s DONE!!! The time taken was %ds %dms\n", name.c_str(), seconds, millis );
    timer.reset();
}