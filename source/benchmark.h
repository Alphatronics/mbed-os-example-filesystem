#ifndef BENCHMARK_H
#define BENCHMARK

#include "Timer.h"
#include <string>

class Benchmark {
public:
    Benchmark();
    virtual ~Benchmark() {}

    void start(std::string& name);
    void stop();

private:
    mbed::Timer timer;
    std::string name;
};

#endif // BENCHMARK_H