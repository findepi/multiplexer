set -ex

gnulib-tool --import --dir=. --lib=libgnu --source-base=lib --m4-base=m4 --doc-base=doc --tests-base=tests --aux-dir=config --libtool --macro-prefix=gl gethostname setenv
autoreconf -iv
cd tools
autoreconf -iv
