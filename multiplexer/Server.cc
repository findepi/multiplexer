#include <asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <asio/ip/tcp.hpp>
//#include "multiplexer/io/MessageHandler.h"
#include "build/multiplexer/Multiplexer.pb.h" /* generated */

#include "Server.h"

using namespace multiplexer;

using std::cerr;
using std::endl;


Server::Server(asio::io_service& io_service, const std::string& host, unsigned short port)
    : Base(io_service)
      // TODO support for name resolving
    , acceptor_(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port))
    //, acceptor_(io_service, asio::ip::tcp::endpoint(
		////asio::ip::address_v4(
		    //*asio::ip::tcp::resolver(io_service).resolve(asio::ip::tcp::resolver_query(host))
		    ////)
		    //,
		//port))
{
}

void Server::start() { _start_accept(); }

void Server::handle_message(Connection::pointer conn, boost::shared_ptr<const RawMessage> raw, boost::shared_ptr<MultiplexerMessage> msg) {
    if (msg->from() == instance_id_) {
	AZOUK_LOG(ERROR, MEDIUMVERBOSITY, CTX("multiplexer.server") TEXT("received message from self")
		FLOW(msg->workflow())
		SKIPFILEIF(!(msg->logging_method() & multiplexer::LoggingMethod::FILE))
	    );
	return;
    }
    return _handle_message(conn, *msg, raw);
}
