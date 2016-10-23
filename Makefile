src/libpbdcex.a: ./src/pbdcex.cpp ./src/pbdcex.h ./src/generater/cpp_gen.cpp ./src/generater/mysql_gen.cpp
	cd src && make libpbdcex.a
src/pbdcexer: ./src/pbdcexer.cpp src/libpbdcex.a
	cd src && make pbdcexer
test/htest: lib/libpbdcex.a bin/pbdcexer test/test.cpp
	cd test && make htest
clean:
	rm -rf include/ lib/ bin/
	cd src && make clean
	cd test && make clean

test: install test/htest
	mv test/htest bin/

install: src/libpbdcex.a src/pbdcexer
	mkdir -p bin
	mkdir -p lib
	mkdir -p include/pbdcex/generater
	mkdir -p include/pbdcex/meta
	mv  src/libpbdcex.a lib/
	mv  src/pbdcexer bin/
	cp -f src/pbdcex.core.hpp include/pbdcex/
	cp -f src/generater/pbdcex.core.hxx include/pbdcex/
	cp -f src/meta/extensions.proto include/pbdcex/
	cp -f src/pbdcex.h include/pbdcex/
	cp -f src/meta/ext_meta.h include/pbdcex/meta/
	cp -f src/generater/cpp_gen.h include/pbdcex/generater/
	cp -f src/generater/mysql_gen.h include/pbdcex/generater/

