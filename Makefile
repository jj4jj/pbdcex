src/libpbdcex.a: ./src/pbdcex.cpp ./src/pbdcex.h
	cd src && make libpbdcex.a
src/pbdcexer: ./src/pbdcexer.cpp src/libpbdcex.a
	cd src && make pbdcexer
test/htest: src/libpbdcex.a src/pbdcexer ./test/test.cpp
	cd src && make pbdcexer
clean:
	rm -rf include/ lib/ bin/
	cd src && make clean
	cd -
	cd test && make clean

test: install test/htest
	mv test/htest bin/

install: src/libpbdcex.a src/pbdcexer
	mkdir -p bin
	mkdir -p lib
	mkdir -p include
	mv src/libpbdcex.a lib/
	mv src/pbdcexer bin/
	cp -f src/pbdcex.core.hpp include/
	cp -f src/generater/pbdcex.core.hxx include/
	cp -f src/meta/extensions.proto include/
	cp -f src/pbdcex.h include/

