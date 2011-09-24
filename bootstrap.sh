set -ex

gnulib-tool --libtool --update
autoreconf -iv
autoreconf -iv tools
