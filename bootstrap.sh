set -ex

gnulib-tool --update
autoreconf -iv
autoreconf -iv tools
