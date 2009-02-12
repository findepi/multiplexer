
#include <boost/python.hpp>

void init_module__logging(); // defined implicitly in azouk/_loggingmodule.cc
void init_module__mxclient(); // defined implicitly in multiplexer/_mxclientmodule.cc

BOOST_PYTHON_MODULE( _allinone ) {
    init_module__logging();
    init_module__mxclient();
}

