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

#ifndef AZLIB_PROTOBUF_STREAM_H
#define AZLIB_PROTOBUF_STREAM_H

#include <unistd.h>
#include <memory>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/stubs/common.h>
#include <boost/noncopyable.hpp>
#include "azlib/util/fd.h"
#include "azlib/repr.h"
#include "azlib/logging.h"

namespace azlib {
    namespace protobuf {

	    struct MessageOutputStream {
		virtual ~MessageOutputStream() {}
		virtual bool write(const google::protobuf::Message& m) = 0;
		virtual void flush() {}

		// helper
		static inline bool Write(const google::protobuf::Message& m,
                        google::protobuf::io::ZeroCopyOutputStream& zcos) {
		    google::protobuf::io::CodedOutputStream coded(&zcos);
#if GOOGLE_PROTOBUF_VERSION >= 2001000
                    coded.WriteVarint32(m.ByteSize());
                    m.SerializeToCodedStream(&coded);
                    return !coded.HadError();
#else
                    // versions prior to 2.1.0 did not have HadError method
                    return coded.WriteVarint32(m.ByteSize()) &&
                        m.SerializeToCodedStream(&coded);
#endif
		}
	    };

	    struct MessageInputStream {
		virtual ~MessageInputStream() {}
		virtual bool read(google::protobuf::Message& m) = 0;

		// helper
		static inline bool Read(google::protobuf::Message& m,
                        google::protobuf::io::CodedInputStream& cis) {
                    boost::uint32_t length;
                    if (!cis.ReadVarint32(&length))
                        return false;
                    if (length < 0)
                        return false;
                    google::protobuf::io::CodedInputStream::Limit limit =
                        cis.PushLimit(length);
                    bool ok = m.ParseFromCodedStream(&cis) &&
                        cis.ConsumedEntireMessage();
                    cis.PopLimit(limit);
                    return ok;
		}
	    };

	    struct OstreamMessageOutputStream : MessageOutputStream {
		OstreamMessageOutputStream(std::ostream* output,
                        bool own_ostream)
		    : output_(output), own_ostream_(own_ostream)
		{}

		virtual bool write(const google::protobuf::Message& m) {
		    google::protobuf::io::OstreamOutputStream oos(output_);
		    return Write(m, oos);
		}

		virtual void flush() {
		    output_->flush();
		}

		~OstreamMessageOutputStream() {
		    if (own_ostream_) {
			delete output_;
		    }
		}
	    private:
		std::ostream* output_;
		bool own_ostream_;
	    };

	    struct FileMessageOutputStream : MessageOutputStream,
                    boost::noncopyable {
		explicit FileMessageOutputStream(int fd, bool own_fd = false)
		    : fd_(fd, own_fd)
		{}

		virtual bool write(const google::protobuf::Message& m) {
		    if (!file_output_stream_)
			file_output_stream_.reset(new google::protobuf::io
                                ::FileOutputStream(fd_.fd()));
		    return Write(m, *file_output_stream_);
		}

		virtual void flush() {
		    file_output_stream_.reset();
		}

	    private:
		util::Fd fd_;
		boost::scoped_ptr<google::protobuf::io::FileOutputStream>
                    file_output_stream_;
	    };

	    struct FileMessageInputStream : MessageInputStream {
		explicit FileMessageInputStream(int fd, bool own_fd = false)
		    : fd_(fd, own_fd)
		    , file_input_stream_(new google::protobuf::io
                            ::FileInputStream(fd_.fd()))
		    , coded_input_stream_(new google::protobuf::io
                            ::CodedInputStream(file_input_stream_.get()))
		{}

		virtual bool read(google::protobuf::Message& m) {
		    return Read(m, *coded_input_stream_);
		}

	    private:
		util::Fd fd_;
                boost::scoped_ptr<google::protobuf::io::FileInputStream>
                    file_input_stream_;
                boost::scoped_ptr<google::protobuf::io::CodedInputStream>
                    coded_input_stream_;
	    };

    }; // namespace protobuf
}; // namespace azlib

#endif
