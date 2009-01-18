
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
