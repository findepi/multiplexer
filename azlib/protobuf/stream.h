#ifndef AZLIB_PROTOBUF_STREAM_H
#define AZLIB_PROTOBUF_STREAM_H

#include <unistd.h>
#include <memory>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_inl.h>
#include <boost/noncopyable.hpp>
#include "azlib/util/fd.h"
#include "azlib/util/str.h"
#include "azlib/logging.h"

namespace azlib {
    namespace protobuf {
	
	    struct MessageOutputStream {
		virtual ~MessageOutputStream() {}
		virtual bool write(const google::protobuf::Message& m) = 0;
		virtual void flush() {}

		// helper
		static inline bool Write(const google::protobuf::Message& m, google::protobuf::io::ZeroCopyOutputStream& zcos) {
		    google::protobuf::io::CodedOutputStream coded(&zcos);
		    m.ByteSize();
		    return google::protobuf::internal::WireFormat::WriteMessage(1, m, &coded);
		}
	    };

	    struct MessageInputStream {
		virtual ~MessageInputStream() {}
		virtual bool read(google::protobuf::Message& m) = 0;

		// helper
		static inline bool Read(google::protobuf::Message& m, google::protobuf::io::CodedInputStream& cis) {
		    typedef boost::uint32_t uint32;
		    boost::uint32_t tag = cis.ReadTag();
		    if (tag == 0)
			return false;
		    if (tag != GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(1, google::protobuf::internal::WireFormat::WIRETYPE_LENGTH_DELIMITED)) {
			AZOUK_LOG(WARNING, MEDIUMVERBOSITY, TEXT("unexpected tag " + str(tag) + "; expecting " + 
				    str(GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(1, google::protobuf::internal::WireFormat::WIRETYPE_LENGTH_DELIMITED))));
			return false;
		    }
		    return google::protobuf::internal::WireFormat::ReadMessage(&cis, &m);
		}
	    };

	    struct OstreamMessageOutputStream : MessageOutputStream {
		OstreamMessageOutputStream(std::ostream* output, bool own_ostream)
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

	    struct FileMessageOutputStream : MessageOutputStream, boost::noncopyable {
		explicit FileMessageOutputStream(int fd, bool own_fd = false)
		    : fd_(fd, own_fd)
		{}

		virtual bool write(const google::protobuf::Message& m) {
		    if (!file_output_stream_)
			file_output_stream_.reset(new google::protobuf::io::FileOutputStream(fd_.fd()));
		    return Write(m, *file_output_stream_);
		}

		virtual void flush() {
		    file_output_stream_.reset();
		}

	    private:
		util::Fd fd_;
		boost::scoped_ptr<google::protobuf::io::FileOutputStream> file_output_stream_;
	    };

	    struct FileMessageInputStream : MessageInputStream {
		explicit FileMessageInputStream(int fd, bool own_fd = false)
		    : fd_(fd, own_fd)
		    , file_input_stream_(new google::protobuf::io::FileInputStream(fd_.fd()))
		    , coded_input_stream_(new google::protobuf::io::CodedInputStream(file_input_stream_.get()))
		{}

		virtual bool read(google::protobuf::Message& m) {
		    return Read(m, *coded_input_stream_);
		}

	    private:
		util::Fd fd_;
		boost::scoped_ptr<google::protobuf::io::FileInputStream> file_input_stream_;
		boost::scoped_ptr<google::protobuf::io::CodedInputStream> coded_input_stream_;
	    };

    }; // namespace protobuf
}; // namespace azlib

#endif
