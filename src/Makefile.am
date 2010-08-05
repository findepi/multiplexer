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
                $(BOOST_DATE_TIME_LDFLAGS)

boost_libs = $(BOOST_PROGRAM_OPTIONS_LIBS) \
             $(BOOST_SIGNALS_LIBS) \
             $(BOOST_SYSTEM_LIBS) \
             $(BOOST_DATE_TIME_LIBS)

AM_LDFLAGS = $(boost_ldflags)

LDADD = $(top_builddir)/lib/libgnu.la $(boost_libs) \
        $(GETHOSTNAME_LIB) $(LIBSOCKET) # gethostname replacement from gnulib

common_sources =    azlib/logging/Logging.pb.cc \
                    azlib/logging/Logging.pb.h \
                    azlib/signals.cc azlib/signals.h \
                    azlib/random.cc azlib/random.h \
                    azlib/logging.cc azlib/logging.h \
                    azlib/util/Assert.cc azlib/util/Assert.h \
                    azlib/util/Exception.cc azlib/util/Exception.h \
                    azouk/util/kwargs.cc azouk/util/kwargs.h \
                    azouk/xrelease.cc azouk/xrelease.h

multiplexer_sources = multiplexer/Multiplexer.pb.cc \
                    multiplexer/Multiplexer.pb.h

multiplexer_other_sources = \
                    multiplexer/BasicClient.cc \
                    multiplexer/BasicClient.h \
                    multiplexer/Client.cc \
                    multiplexer/Client.h \
                    multiplexer/Server.cc \
                    multiplexer/Server.h \
                    multiplexer/ConnectionsManager.cc \
                    multiplexer/ConnectionsManager.h \
                    multiplexer/io/RawMessage.cc \
                    multiplexer/io/RawMessage.h \
                    multiplexer/backend/BaseMultiplexerServer.cc \
                    multiplexer/backend/BaseMultiplexerServer.h

mxcontrol_SOURCES = mxcontrol.cc $(common_sources) \
                    mxcontrol-commands/TasksHolder.cc \
                    mxcontrol-commands/TasksHolder.h \
                    mxcontrol-commands/Task.cc \
                    mxcontrol-commands/Task.h \
                    mxcontrol-commands/StreamLogs.cc \
                    mxcontrol-commands/StreamLogs.h \
                    mxcontrol-commands/StartMultiplexerServer.cc \
                    mxcontrol-commands/StartMultiplexerServer.h \
                    mxcontrol-commands/ReceiveLogs.cc \
                    mxcontrol-commands/ReceiveLogs.h \
                    mxcontrol-commands/Help.cc \
                    mxcontrol-commands/Help.h \
                    $(multiplexer_sources) \
		    $(multiplexer_other_sources) \
                    multiplexer/multiplexer.constants.h \
                    multiplexer.rules \
                    azouk/logging/type_id_constants.h \
                    logging.type_id.constants.txt

server_SOURCES =    server.cc $(common_sources)

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
                azouk/logging/type_id_constants.py

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

BUILT_SOURCES = azlib/logging/Logging.pb.cc azlib/logging/Logging.pb.h \
		azlib/logging/Logging_pb2.py \
                multiplexer/Multiplexer.pb.cc multiplexer/Multiplexer.pb.h \
		multiplexer/Multiplexer_pb2.py \
                multiplexer/multiplexer.constants.h \
                multiplexer/multiplexer_constants.py \
                azouk/logging/type_id_constants.h \
                azouk/logging/type_id_constants.py

multiplexer/multiplexer.constants.h multiplexer/multiplexer_constants.py: \
      multiplexer.rules ../tools/generate_constants
	$(AM_V_GEN)../tools/generate_constants $< $@

../tools/generate_constants:
	$(MAKE) -C ../tools generate_constants

azouk/logging/type_id_constants.py: logging.type_id.constants.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(V_COPY)cp $< $@

# version file generation
version = unknown
version_hash = $(shell echo -n "$(version)" | sha1sum - | cut -d' ' -f1)
version_short_hash = $(shell echo $(version_hash) | cut --bytes=1-7)

file_token = $(firstword $(shell echo $@ | sha1sum))
azouk/logging/type_id_constants.h: logging.type_id.constants.txt
	$(AM_V_at)mkdir -p $(dir $@)
	$(AM_V_at)echo "#ifndef $(file_token)" > $@
	$(AM_V_at)echo "#define $(file_token)" >> $@
	$(AM_V_at)echo "namespace type_id_constants {" >> $@
	$(AM_V_GEN)awk '/^[^#]/ {print "\tstatic const unsigned int", $$0 ";" }' $< >> $@
	$(AM_V_at)echo "};" >> $@
	$(AM_V_at)echo "#endif" >> $@
