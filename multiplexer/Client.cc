
#include "build/multiplexer/Multiplexer.pb.h"
#include "Client.h"

using namespace multiplexer;
using std::cerr;

Client::Client(boost::uint32_t client_type)
    : io_service_ptr_(new asio::io_service())
    , io_service_(*io_service_ptr_)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::Client(boost::shared_ptr<asio::io_service> io_service, boost::uint32_t client_type)
    : io_service_ptr_(io_service)
    , io_service_(*io_service_ptr_)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::Client(asio::io_service& io_service, boost::uint32_t client_type)
    : io_service_(io_service)
    , basic_client_(BasicClient::Create(io_service_, client_type))
{}

Client::~Client() {
    shutdown();
}


