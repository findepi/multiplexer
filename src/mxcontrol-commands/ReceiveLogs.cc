//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//

#include "config.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "mxcontrol-commands/Task.h"
#include "mxcontrol-commands/TasksHolder.h"
#include "azlib/logging.h"
#include "azlib/protobuf/stream.h"
#include "multiplexer/Multiplexer.pb.h" /* generated */
#include "multiplexer/Client.h"
#include "multiplexer/backend/BaseMultiplexerServer.h"

using namespace std;
using namespace azlib;
using namespace azlib::logging;
using namespace multiplexer;

using azouk::util::kwargs::Kwargs;

namespace mxcontrol {

    class ReceiveLogs : public Task {
    public:
	virtual int run();
	virtual std::string short_description() const {
	    return "receive logs and print them on stdout";
	}
	virtual void print_help(std::ostream& out) const {
	    out << "Start a Multiplexer backend that does something with logs.\n";
	}

    protected:
	virtual void _initialize_options_description(
		po::options_description& generic) {
	    _add_multiplexer_client_options(generic);
	}

    private:
    };

    REGISTER_MXCONTROL_SUBCOMMAND(receivelogs, mxcontrol::ReceiveLogs);


    struct LogReceiverServer : public
			       multiplexer::backend::BaseMultiplexerServer {

	LogReceiverServer(multiplexer::Client* conn,
		multiplexer::backend::PeerType type)
	    : BaseMultiplexerServer(conn, type)
	{}

    protected:
	virtual void handle_message(MultiplexerMessage& mxmsg) {
	    std::string tf;
	    google::protobuf::TextFormat::PrintToString(mxmsg, &tf);
	    std::cout << tf << "\n";
	    no_response();

	    send_message(Kwargs()
		    .set("message", std::string())
		    .set("type", types::PING)
		    .set("to", mxmsg.from())
		);
	}
    };

    int ReceiveLogs::run() {

	LogReceiverServer(&_multiplexer_client(peers::LOG_RECEIVER_EXAMPLE),
		peers::LOG_RECEIVER_EXAMPLE).serve_forever();

	return 0;
    }


}; // namespace mxcontrol

