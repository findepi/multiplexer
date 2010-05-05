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

#ifndef MX_CONFIG_H
#define MX_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "multiplexer/Multiplexer.pb.h" /* generated */
#include "azlib/util/Exception.h"
#include "azlib/util/Assert.h"

#ifndef IS_GENERATE_CONSTANTS
#include "azlib/logging.h"
#endif

namespace multiplexer {

    namespace config_detail {
	    static const char* const BASIC_CONFIGURATION_DATA = "\
peer {\
    type: 1\
    name: \"MULTIPLEXER\"\
    comment: \"Peer type representing normal multiplexer instance.\"\
}\
type {\
    type: 2\
    name: \"CONNECTION_WELCOME\"\
    comment: \"message interchange by peers just after connecting to each other\"\
}\
";
    };

    template <
	    template <typename, typename, typename, typename> class map_template_ = std::map
	>
    class Config {
	/* static members */
	public:
	    struct Exception : public azlib::Exception {
		Exception(const std::string& explanation) : azlib::Exception(explanation)
		{}
	    };

	public:
	    Config()
		: initialized_(false)
		, unkown_("UNKNOWN")
	    {
		read_configuration(config_detail::BASIC_CONFIGURATION_DATA);
	    }

	    Config(const std::string& file)
		: initialized_(true)
		, unkown_("UNKNOWN")
	    {
		read_configuration(file);
	    }

	    void clear() {
		*this = Config();
		Assert(!initialized());
	    }

	    void read_configuration(const std::string& file) {
		using std::cout;
		using std::ifstream;

		MultiplexerRules rules;

#ifndef IS_GENERATE_CONSTANTS
		using namespace azlib::logging::consts;
		AZOUK_LOG(DEBUG, MEDIUMVERBOSITY, CTX("config")
                        TEXT("reading configuration file"));
#endif

		ifstream in(file.c_str(), ifstream::in | ifstream::binary);
		if (!in) {
		    MXTHROW(azlib::Exception("There is no config file '" + file + "'"));
		}

		google::protobuf::io::IstreamInputStream zcis(&in);
		bool ok = google::protobuf::TextFormat::Parse(&zcis, &rules);
		AssertMsg(ok, "ParseFromIstream failed");
		Assert(in.eof());

		read_configuration(rules);
		initialized_ = true;
	    }

	    bool inline initialized() const { return initialized_; }

	    // make 4-argument map_template_ 2-argument
	    template <typename Key, typename Data> struct map_template {
		typedef map_template_<Key, Data,
			std::less<Key>, std::allocator<std::pair<const Key, Data> > > type;
	    };

	    typedef typename map_template<boost::uint32_t, MultiplexerMessageDescription>::type MessageDescriptionById;
	    typedef typename map_template<std::string, MultiplexerPeerDescription>::type PeerDescriptionByName;
	    typedef typename map_template<boost::uint32_t, MultiplexerPeerDescription>::type PeerDescriptionById;

	    const MessageDescriptionById& message_description_by_id() const { return message_description_by_id_; }
	    const PeerDescriptionByName& peer_by_name() const { return peer_by_name_; }
	    const PeerDescriptionById& peer_by_type() const { return peer_by_type_; }

	    /* shortcuts */
	    // you must know this type is valid
	    const std::string& message_name_by_type(boost::uint32_t type) const {
		typename MessageDescriptionById::const_iterator i = message_description_by_id_.find(type);
		Assert(!initialized() || i != message_description_by_id_.end());
		return i != message_description_by_id_.end() ? i->second.name() : unkown_;
	    }

	    // you must know this type is valid
	    const std::string& peer_name_by_type(boost::uint32_t type) const {
		typename PeerDescriptionById::const_iterator i = peer_by_type_.find(type);
		Assert(!initialized() || i != peer_by_type_.end());
		return i != peer_by_type_.end() ? i->second.name() : unkown_;
	    }

	    // Config's ownership
	    const MultiplexerPeerDescription* peer_description(boost::uint32_t type) const {
		return _valueptr_or_null(peer_by_type_, type);
	    }

	    // Config's ownership
	    const MultiplexerMessageDescription* message_description(boost::uint32_t type) const {
		return _valueptr_or_null(message_description_by_id_, type);
	    }

	private:
	    template <typename Key, typename Value, typename Compare, typename Alloc>
	    const Value* _valueptr_or_null(const map_template_<Key, Value, Compare, Alloc>& m, const Key& k) const {
		typename map_template_<Key, Value, Compare, Alloc>::const_iterator mi = m.find(k);
		return mi == m.end() ? NULL : &mi->second;
	    }

	    /**
	     * __insert(map, key, data)
	     *	    inserts a copy of `data' into a map or multimap `map' at key `key'
	     *	    returns a reference to the inserted copy
	     */
	    template <typename Key, typename Data, typename Compare, typename Alloc>
	    Data& __insert(std::map<Key, Data, Compare, Alloc>& m, const Key& k, const Data& d) const {
		return m.insert(std::make_pair(k, d)).first->second;
	    }

	    template <typename Key, typename Data, typename Compare, typename Alloc>
	    Data& __insert(std::multimap<Key, Data, Compare, Alloc>& m, const Key& k, const Data& d) const {
		return m.insert(std::make_pair(k, d))->second;
	    }

	    void read_configuration(const MultiplexerRules& rules) {
		// validate the rules
		BOOST_FOREACH(const MultiplexerMessageDescription& mtmp, rules.type()) {
		    BOOST_FOREACH(const MultiplexerMessageDescription::RoutingRule& rr, mtmp.to()) {
			if (!rr.has_peer()) {
			    std::cerr << "ERROR: MultiplexerMessageDescription::RoutingRule without peer name\n";
			    return;
			}
		    }
		}

		BOOST_FOREACH(const MultiplexerPeerDescription& m, rules.peer()) {
		    /* Multiplexer peer description */
		    peer_by_type_.insert(std::make_pair(m.type(), m));
		    peer_by_name_.insert(std::make_pair(m.name(), m));
		}

		BOOST_FOREACH(const MultiplexerMessageDescription& mtmp, rules.type()) {
		    /* package descirption with routing rules definitions */
		    MultiplexerMessageDescription& m =
			__insert(message_description_by_id_, mtmp.type(), mtmp);

		    /* routing rules */
		    BOOST_FOREACH(MultiplexerMessageDescription::RoutingRule& rr, *m.mutable_to()) {
			std::map<std::string, MultiplexerPeerDescription>::iterator peerp = peer_by_name_.find(rr.peer());
			if (peerp == peer_by_name_.end())
			    MXTHROW(typename Config::Exception("Unknown peer definition: '" + rr.peer() + "'"));

			MultiplexerPeerDescription& peer = peerp->second;
			rr.set_peer_type(peer.type());
			//rr.clear_peer();
		    }
		}
	    }
	    void read_configuration(const char* configuration_data) {
		MultiplexerRules rules;
		bool ok = google::protobuf::TextFormat::ParseFromString(configuration_data, &rules);
		AssertMsg(ok, "ParseFromIstream failed");
		read_configuration(rules);
	    }

	private:
	    bool initialized_;
	    std::string unkown_;
	    MessageDescriptionById message_description_by_id_;
	    PeerDescriptionByName peer_by_name_;
	    PeerDescriptionById peer_by_type_;
    };

}; // namespace multiplexer

#endif
