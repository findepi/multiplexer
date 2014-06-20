## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I ../m4 --install

include $(top_srcdir)/../lib/Makefile_protoc
include $(top_srcdir)/../lib/Makefile_v_copy

generate_constants_SOURCES = generate_constants.cc \
       multiplexer/Multiplexer.pb.cc multiplexer/Multiplexer.pb.h \
       azlib/logging/Logging.pb.cc azlib/logging/Logging.pb.h \
       ../src/azlib/util/Assert.cc ../src/azlib/util/Assert.h \
       ../src/azlib/util/Exception.cc ../src/azlib/util/Exception.h

noinst_PROGRAMS = generate_constants

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I$(top_builddir)
AM_CPPFLAGS += -I$(top_srcdir)/../src -I$(top_builddir)/../src

BUILT_SOURCES = azlib/logging/Logging.pb.cc azlib/logging/Logging.pb.h \
                azlib/logging/Logging.proto \
                multiplexer/Multiplexer.pb.cc multiplexer/Multiplexer.pb.h \
		multiplexer/Multiplexer.proto

azlib/logging/Logging.proto: ../src/azlib/logging/Logging.proto
	$(AM_V_at)$(MKDIR_P) azlib/logging
	$(V_COPY)cp $< $@

multiplexer/Multiplexer.proto: ../src/multiplexer/Multiplexer.proto
	$(AM_V_at)$(MKDIR_P) multiplexer
	$(V_COPY)cp $< $@
