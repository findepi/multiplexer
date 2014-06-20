## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = lib src
EXTRA_DIST = bootstrap.sh \
        m4/gnulib-cache.m4 \
        tools/generate_constants.cc \
        tools/configure.ac tools/configure \
        tools/Makefile.am tools/Makefile.in \
        tools/aclocal.m4 \
        tools/config/config.h.in \
        tools/config/depcomp \
        tools/config/missing \
        tools/config/install-sh \
        multiplexer.rules

DISTCLEANFILES = confdefs.h

# http://www.gnu.org/software/libtool/manual/libtool.html#LT_INIT
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

distclean-local:
	$(MAKE) -C tools distclean

clean-local:
	$(MAKE) -C tools clean

maintainerclean-local:
	$(MAKE) -C tools maintainerclean
