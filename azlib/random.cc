
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <fstream>

#include "random.h"

using namespace azlib;

AutoSeedingRand48::AutoSeedingRand48()
{
    std::ifstream rf("/dev/urandom", std::ifstream::binary | std::ifstream::in);
    boost::uint64_t seed = getpid();
    rf.read((char*) &seed, sizeof(seed));
    this->seed(seed);
}
