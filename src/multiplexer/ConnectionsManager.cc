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


#include "build/multiplexer/multiplexer.constants.h" /* generated */

#include "ConnectionsManager.h"

namespace multiplexer {
    namespace impl {

	boost::shared_ptr<const RawMessage> create_welcome_message(
		boost::uint32_t peer_type,
		boost::uint64_t instance_id,
		const std::string& multiplexer_password) {

	    // prepare WelcomeMessage
	    WelcomeMessage welcome;
	    welcome.set_type(peer_type);
	    welcome.set_id(instance_id);
	    welcome.set_multiplexer_password(multiplexer_password);

	    // pack it into MultiplexerMessage
	    MultiplexerMessage msg;
	    msg.set_id(0);
	    msg.set_from(instance_id);
	    msg.set_type(types::CONNECTION_WELCOME);
	    welcome.SerializeToString(msg.mutable_message());

	    // serialize
	    return boost::shared_ptr<const RawMessage>(RawMessage::FromMessage(msg));
	}

    }; // namespace impl
}; // namespace multiplexer
