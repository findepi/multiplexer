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

#ifndef MX_RAWMESSAGE_H
#define MX_RAWMESSAGE_H

#include <string>
#include <list>
#include <boost/asio/buffer.hpp>
#include <google/protobuf/message.h>

#include "multiplexer/configuration.h"

namespace multiplexer {

    /**
     * RawMessage is a structured representation of wire format [length, crc, body].
     * It has well defined life cycles:
     *	    create empty -> read -> freeze -> use for writing
     *	    create frozen with message ->  use for writing
     * so that we don't have to serialize back the message after it's deserialized
     * -- save on Protocol Buffers serialization -- prior to forwarding it to other peers.
     */
    class RawMessage {
	public:
	    enum Usability { READING, WRITING, NONE, PRE_WRITING };
	    static const boost::uint32_t HEADER_LENGTH = 8; // for length_ and crc32_

	    // construct message suitable for reading input with ASIO
	    inline RawMessage()
		: usability_(READING)
		, length_(0), crc32_(0)
		, header_(HEADER_LENGTH, 0)
	    {
		BOOST_STATIC_ASSERT((HEADER_LENGTH == sizeof(length_) + sizeof(crc32_)));
	    }

	    // construct message suitable for writing output with ASIO
	    inline explicit RawMessage(const std::string& message)
		: usability_(PRE_WRITING)
		, length_(message.size())
		, crc32_(Crc32(message))
		, header_(HEADER_LENGTH, 0)
		, contents_(message)
	    {
		Assert(contents_.size() <= MAX_MESSAGE_SIZE);
		initialize_header();
		switch_to_writing();
	    }

	    // like above but destroys `contents'
	    inline explicit RawMessage(std::string* message)
		: usability_(PRE_WRITING)
		, length_(message->size())
		, crc32_(Crc32(*message))
		, header_(HEADER_LENGTH, 0)
		, contents_()
	    {
		contents_.swap(*message);
		Assert(contents_.size() <= MAX_MESSAGE_SIZE);
		initialize_header();
		switch_to_writing();
	    }

	    static RawMessage* FromMessage(const ::google::protobuf::Message& mxmsg) {
		std::string serialized;
		mxmsg.SerializeToString(&serialized);
		return new RawMessage(&serialized);
	    }

	    /* accessors */
	    inline Usability usability() const { return usability_; }
	    inline const std::string& get_message() const { return contents_; }

	    /* ASIO reading buffers (for reading RawMessage from channel) */
	    // returns buffer for reading-in RawMessage header
	    inline boost::asio::mutable_buffer get_header_buffer() {
		Assert(usability_ == READING);
		return boost::asio::buffer((void*)(header_.size() ? &header_[0] : NULL), header_.size());
	    }
	    inline size_t get_header_length() const { return header_.size(); }

	    // returns buffer for reading-in RawMessage body
	    inline boost::asio::mutable_buffer get_body_buffer() {
		Assert(usability_ == READING);
		return boost::asio::buffer((void*)(contents_.size() ? &contents_[0] : NULL), contents_.size());
	    }

	    // returns RawMessage body length
	    inline size_t get_body_length() const {
		Assert((contents_.empty() && !length_) || length_ == contents_.size());
		return length_;
	    }

	    /* ASIO writing buffers (for writing RawMessage to channel) */
	    // returns buffer for writing-out whole RawMessage (header + body)
	    inline const std::list<boost::asio::const_buffer>& get_message_buffer() const {
		Assert(usability_ == WRITING);
		Assert(writing_buffers_.size());
		return writing_buffers_;
	    }

	    /* header manipulations */
	    // call this before calling get_body_buffer()
	    bool unpack_header();
	    // call this before calling get_message() or get_message_buffer()
	    bool verify();

	private:
	    static boost::uint32_t Crc32(const std::string& message);
	    void initialize_header();
	    void switch_to_writing();

	private:
	    Usability usability_;
	    boost::uint32_t length_, crc32_;
	    std::string header_;
	    std::string contents_;
	    std::list<boost::asio::const_buffer> writing_buffers_; // buffers that can be used in write operations
    };

}; // namespace multiplexer

#endif
