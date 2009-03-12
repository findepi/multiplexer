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


#include <boost/crc.hpp>
#include <boost/static_assert.hpp>

#include "azlib/util/Assert.h"
#include "azlib/encodings/LittleEndian.h"
#include "azlib/encoders/EncodeToRange.h"
#include "azlib/encoders/DecodeFromRange.h"

#include "RawMessage.h"

using namespace multiplexer;

void RawMessage::switch_to_writing() {
    Assert(usability_ == PRE_WRITING || usability_ == READING);
    Assert(writing_buffers_.empty());
    usability_ = READING; // fool get_header_buffer() and get_body_buffer()
    writing_buffers_.push_back(get_header_buffer());
    writing_buffers_.push_back(get_body_buffer());
    usability_ = WRITING;
}

bool RawMessage::unpack_header() {
    Assert(usability_ == READING);

    azlib::encoders::DecodeFromRange<std::string::const_iterator, azlib::encodings::LittleEndian> decoder(header_.begin(), header_.end());
    decoder(length_);
    decoder(crc32_);
    if (length_ > MAX_MESSAGE_SIZE)
	return false;

    // allocate memory for reading
    Assert(contents_.empty());
    contents_.resize(length_);

    // indicate success
    return true;
}

bool RawMessage::verify() {
    Assert(usability_ == READING);
    AssertMsg(!contents_.empty(), "You can't verify partial message.");
    const bool ok = Crc32(contents_) == crc32_;
    if (!ok) {
	usability_ = NONE;
    } else {
	usability_ = PRE_WRITING;
	switch_to_writing();
    }
    return ok;
}

boost::uint32_t RawMessage::Crc32(const std::string& message) {
    boost::crc_32_type crc;
    crc.process_block(&*message.begin(), &*message.end());
    return crc.checksum();
}

void RawMessage::initialize_header() {
    azlib::encoders::EncodeToRange<std::string::iterator, azlib::encodings::LittleEndian> encoder(header_.begin(), header_.end());
    encoder(length_);
    encoder(crc32_);
}
