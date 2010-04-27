bin_PROGRAMS = mxcontrol server generate_constants
AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I$(top_builddir)
AM_CPPFLAGS += -I$(top_builddir)/lib -I$(top_srcdir)/lib

GENERALFLAGS = -Wall -Wnon-virtual-dtor -Winit-self \
               -Wswitch-enum -Wunused -Wunused-parameter \
               -DBOOST_DISABLE_THREADS

AM_CXXFLAGS = -O3 $(GENERALFLAGS)

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

server_SOURCES =    server.cc $(common_sources)

generate_constants_SOURCES = generate_constants.cc $(common_sources) \
                    $(multiplexer_sources)

BUILT_SOURCES = azlib/logging/Logging.pb.cc azlib/logging/Logging.pb.h \
                multiplexer/Multiplexer.pb.cc multiplexer/Multiplexer.pb.h

%.pb.h %.pb.cc: %.proto
	$(PROTOC) $(PROTOCFLAGS) -I $(top_srcdir)/azouk --cpp_out=$(builddir) $<


# version file generation
version = unknown
version_hash = $(shell echo -n "$(version)" | sha1sum - | cut -d' ' -f1)
version_short_hash = $(shell echo $(version_hash) | cut --bytes=1-7)
