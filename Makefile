# Makefile wrapper for waf

all:
	CXXFLAGS="-Wall" ./waf configure --enable-examples --enable-tests
	./waf

# free free to change this part to suit your requirements
configure:
	./waf configure --enable-examples --enable-tests

build:
	./waf build

install:
	./waf install

clean:
	./waf clean

distclean:
	./waf distclean
