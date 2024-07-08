#include <iostream>
#include <fstream>
#include <limits> 
#include <random>
#include <chrono>

int main () {
    int size_of_file = 0;
    double lower_bound = std::numeric_limits<double>::min();
    double upper_bound = std::numeric_limits<double>::max();
    std::uniform_real_distribution<double> distribution(lower_bound, upper_bound);

    typedef std::chrono::high_resolution_clock myclock;
    auto begin = myclock::now();
    auto duration = myclock::now() - begin;
    unsigned seed = duration.count() * 1024;

    std::default_random_engine generator;
    generator.seed(seed);
    std::ofstream fout("unsorted_double.txt");

    while(size_of_file < 1024*1024*1024) {
        double randomValue = distribution(generator);
        fout << randomValue << '\n';
        size_of_file = fout.tellp();
    }

    fout.close(); 
    
    return 0;
}
