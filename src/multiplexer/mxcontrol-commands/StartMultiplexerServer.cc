
#include <asio.hpp>
#include "multiplexer/Server.h"
#include "azlib/util/str.h"
#include "mxcontrol-commands/StartMultiplexerServer.h"
#include "mxcontrol-commands/TasksHolder.h"

namespace mxcontrol {

    int StartMultiplexerServer::run() {
	using std::string;
	using azlib::str;

	string host = host_port_;
	boost::uint16_t port = 1980;

	string::size_type colonpos = host_port_.find(':');
	Assert(colonpos < host_port_.size() || colonpos == string::npos);

	if (colonpos < host_port_.size()) {
	    // port specified
	    Assert(host_port_[colonpos] == ':');
	    string(&host_port_[0], &host_port_[colonpos]).swap(host);
	    string portstring(&host_port_[0] + colonpos + 1, &host_port_[0] + host_port_.size());
	    AssertMsg(portstring.find(':') == string::npos, "Invalid address spec: two colons");
	    port = boost::lexical_cast<boost::uint16_t>(portstring);
	}

	// TODO support for name resolving (e.g. host = "localhost" by default)
	asio::io_service io_service;
	multiplexer::Server::pointer server = multiplexer::Server::Create(io_service, host, port);
	server->clear_rules();
	server->read_rules(rules_file_);
	server->set_multiplexer_password(multiplexer_password_);
	server->start();
	AZOUK_LOG(INFO, LOWVERBOSITY, TEXT("starting MX server on " + host + ":" + str(port)));
	io_service.run();

	return 0;
    }
};

REGISTER_MXCONTROL_SUBCOMMAND(run_multiplexer, mxcontrol::StartMultiplexerServer);
