# Keep everything that is generated.
.SECONDARY:
# Remove works-in-progress.
.DELETE_ON_ERROR:

include $(top_srcdir)/lib/Makefile_protoc
include $(top_srcdir)/lib/Makefile_v_copy

bin_PROGRAMS = mxcontrol server
AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I$(top_builddir)
AM_CPPFLAGS += -I$(top_builddir)/lib -I$(top_srcdir)/lib

GENERALFLAGS = -Wall -Wnon-virtual-dtor -Winit-self \
               -Wswitch-enum -Wunused -Wunused-parameter \
               -DBOOST_DISABLE_THREADS

AM_CXXFLAGS = -O3 $(GENERALFLAGS)
AM_CFLAGS = -O3 $(GENERALFLAGS)

boost_ldflags = $(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
                $(BOOST_SIGNALS_LDFLAGS) \
                $(BOOST_SYSTEM_LDFLAGS) \
                $(BOOST_DATE_TIME_LDFLAGS) \
                $(BOOST_FILESYSTEM_LDFLAGS)

boost_libs = $(BOOST_PROGRAM_OPTIONS_LIBS) \
             $(BOOST_SIGNALS_LIBS) \
             $(BOOST_SYSTEM_LIBS) \
             $(BOOST_DATE_TIME_LIBS) \
             $(BOOST_FILESYSTEM_LIBS)

AM_LDFLAGS = $(boost_ldflags)

LDADD = $(top_builddir)/lib/libgnu.la $(boost_libs) \
        $(GETHOSTNAME_LIB) $(LIBSOCKET) # gethostname replacement from gnulib

common_sources =    azlib/logging/Logging.pb.cc \
		    azlib/signals.cc \
		    azlib/random.cc  \
		    azlib/logging.cc \
		    azouk/xrelease.h \
		    azlib/util/Assert.cc \
		    azlib/util/Exception.cc \
		    azouk/util/kwargs.cc \
		    azouk/xrelease.cc \
		    azlib/logging/Logging.proto

multiplexer_sources = multiplexer/Multiplexer.pb.cc

nobase_include_HEADERS = \
		    multiplexer/Multiplexer.pb.h \
		    multiplexer/multiplexer.constants.h \
		    multiplexer/BasicClient.h \
		    multiplexer/Client.h \
		    multiplexer/Server.h \
		    multiplexer/ConnectionsManager.h \
		    multiplexer/io/RawMessage.h \
		    multiplexer/backend/BaseMultiplexerServer.h \
		    multiplexer/Config.h \
		    multiplexer/configuration.h \
		    multiplexer/io/Connection.h \
		    azlib/encoders/BaseDecoder.h \
		    azlib/encoders/BaseEncoder.h \
		    azlib/encoders/DecodeFromRange.h \
		    azlib/encoders/EncodeToOutputIterator.h \
		    azlib/encoders/EncodeToRange.h \
		    azlib/encodings/LittleEndian.h \
		    azlib/util/Exception.h \
		    azouk/util/kwargs.h \
		    azlib/logging/Logging.pb.h \
		    azlib/logging.h azlib/random.h  azlib/signals.h \
		    azlib/logging/impl.h \
		    azlib/util/Assert.h \
		    azlib/logging/log_tokens.h \
		    azlib/macros.h \
		    azlib/preproc/common.h \
		    azlib/preproc/create_message/detail.h \
		    azlib/preproc/create_message.h \
		    azlib/preproc/kwargs.h \
		    azlib/program.h \
		    azlib/protobuf/stream.h \
		    azlib/repr.h azlib/spanset.h azlib/str.h \
		    azlib/timer.h azlib/triple.h \
		    azlib/util/fd.h azlib/util/functors.h \
		    azlib/util/IntrusiveValue.h \
		    azlib/util/type_functors.h azlib/util/type_utils.h \
		    azlib/vector.h \
		    azlib/initialization.h

multiplexer_other_sources = \
		    multiplexer/BasicClient.cc \
		    multiplexer/Client.cc \
		    multiplexer/Server.cc \
		    multiplexer/ConnectionsManager.cc \
		    multiplexer/io/RawMessage.cc \
		    multiplexer/backend/BaseMultiplexerServer.cc

mxcontrol_SOURCES = mxcontrol.cc $(common_sources) \
		    mxcontrol-commands/TasksHolder.cc \
		    mxcontrol-commands/TasksHolder.h \
		    mxcontrol-commands/Task.cc \
		    mxcontrol-commands/Task.h \
		    mxcontrol-commands/StreamLogs.cc \
		    mxcontrol-commands/StartMultiplexerServer.cc \
		    mxcontrol-commands/StartMultiplexerServer.h \
		    mxcontrol-commands/ReceiveLogs.cc \
		    mxcontrol-commands/Help.cc \
		    mxcontrol-commands/Help.h \
		    $(multiplexer_sources) \
		    $(multiplexer_other_sources) \
		    azouk/logging/type_id_constants.h \
		    logging.type_id.constants.txt

server_SOURCES =    server.cc $(common_sources)

BUILT_SOURCES = azlib/logging/Logging.pb.cc azlib/logging/Logging.pb.h \
                multiplexer/Multiplexer.pb.cc multiplexer/Multiplexer.pb.h \
                multiplexer/multiplexer.constants.h \
                azouk/logging/type_id_constants.h

if DO_PYTHON
nobase_python_PYTHON = azlib/__init__.py \
                azlib/logging/__init__.py \
                azouk/__init__.py \
                azouk/logging/__init__.py \
                azouk/logging/collector.py \
                azouk/logging/django_termcolors.py \
                azouk/logging/streaming.py \
                azlib/logging/Logging_pb2.py \
                azouk/services/__init__.py \
                azouk/services/collectors/__init__.py \
                azouk/services/collectors/events.py \
                azouk/util/Timer.py \
                azouk/util/__init__.py \
                azouk/util/decorators.py \
                multiplexer/__init__.py \
                multiplexer/clients.py \
                multiplexer/mxclient.py \
                multiplexer/protocolbuffers.py \
                multiplexer/test/__init__.py \
                multiplexer/test/test_mxclient.py \
                multiplexer/test/test_simple_connection.py \
                multiplexer/multiplexer_constants.py \
                multiplexer/Multiplexer_pb2.py \
                azouk/logging/type_id_constants.py \
                azouk/release.py

nobase_pyexec_LTLIBRARIES = azouk/_allinone.la
azouk__allinone_la_SOURCES = \
                azouk/_allinonemodule.cc \
                azouk/_loggingmodule.cc \
                multiplexer/_mxclientmodule.cc \
                $(common_sources) \
                $(multiplexer_other_sources) \
                $(multiplexer_sources)

azouk__allinone_la_LDFLAGS = -avoid-version -module $(BOOST_PYTHON_LDFLAGS)
azouk__allinone_la_LIBADD = $(LDADD) $(BOOST_PYTHON_LIBS)
azouk__allinone_la_CPPFLAGS = $(PYTHON_INCLUDES) $(AM_CPPFLAGS)

BUILT_SOURCES += azlib/logging/Logging_pb2.py \
                multiplexer/Multiplexer_pb2.py \
                multiplexer/multiplexer_constants.py \
                azouk/logging/type_id_constants.py \
                azouk/release.h azouk/release.py
endif

# this is used by generate_constants
EXTRA_DIST = azlib/hash_set.h \
             multiplexer.rules

generator = ../tools/generate_constants

_rules_prefix = $(if $(subst $(RULES),,$(abspath $(RULES))),../,)
rules = $(_rules_prefix)$(RULES)

multiplexer/multiplexer.constants.h multiplexer/multiplexer_constants.py: $(rules) $(generator)
	$(AM_V_GEN)$(generator) $< $@

$(generator):
	$(MAKE) -C ../tools generate_constants

azouk/logging/type_id_constants.py: logging.type_id.constants.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(V_COPY)cp $< $@

file_token = $(firstword $(shell echo $@ | sha1sum))
azouk/logging/type_id_constants.h: logging.type_id.constants.txt
	$(AM_V_at)mkdir -p $(dir $@)
	$(AM_V_at)echo "#ifndef $(file_token)" > $@
	$(AM_V_at)echo "#define $(file_token)" >> $@
	$(AM_V_at)echo "namespace type_id_constants {" >> $@
	$(AM_V_GEN)awk '/^[^#]/ {print "\tstatic const unsigned int", $$0 ";" }' $< >> $@
	$(AM_V_at)echo "};" >> $@
	$(AM_V_at)echo "#endif" >> $@

# version file generation
version = unknown
version_hash = $(shell echo -n "$(version)" | $(SHA1SUM) - | $(CUT) -d' ' -f1)
version_short_hash = $(shell echo $(version_hash) | $(CUT) --bytes=1-7)

RELEASE_H_TOKEN = AZOUK_RELEASE_H

azouk/release.h:
	$(MKDIR_P) "$(dir $@)"
	$(AM_V_GEN){ \
            echo "#ifndef $(RELEASE_H_TOKEN)"; \
            echo "#define $(RELEASE_H_TOKEN)"; \
            echo "namespace azouk { namespace release {"; \
            echo "      const char* const version = \"$(version)\";"; \
            echo "      const char* const version_hash = \"$(version_hash)\";"; \
            echo "      const char* const version_short_hash = \"$(version_short_hash)\";"; \
            echo "} /* release */ } /* azouk */"; \
            echo "#endif"; \
        } > $@

azouk/release.py:
	$(MKDIR_P) "$(dir $@)"
	$(AM_V_GEN){ \
            echo "version = \"$(version)\""; \
            echo "version_hash = \"$(version_hash)\""; \
            echo "version_short_hash = \"$(version_short_hash)\""; \
        } > $@
