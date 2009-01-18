
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
