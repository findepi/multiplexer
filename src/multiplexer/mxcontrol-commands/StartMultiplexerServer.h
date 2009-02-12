
#include "mxcontrol-commands/Task.h"

namespace mxcontrol {

    class StartMultiplexerServer : public Task {
    public:
	virtual int run();
	virtual std::string short_synopsis(const std::string& commandname) { return "<" + commandname + "-options> [--address] address:port"; }

    protected:
	virtual void _initialize_options_description(po::options_description& generic) {
	    generic.add_options()
		("rules", po::value(&rules_file_)
			->default_value("multiplexer.rules"),
		    "file from which routing rules will be read")
		("address,M", po::value(&host_port_)
			->default_value("0.0.0.0:1980"),
		    "local address to listen on")
		("multiplexer-password,p", po::value(&multiplexer_password_),
		    "password required when authorizing new connections")
		;
	}

	virtual void _initialize_positional_options_description(po::positional_options_description& positional) {
	    positional.add("address", 1);
	}

    private:
	std::string host_port_;
	std::string rules_file_;
    };

};
