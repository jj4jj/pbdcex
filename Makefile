
pbdcexer: pbdcexer.cpp libpbcex.a ../cxxtemplates/libxctmp.a
	g++ -o $@ $^ -std=c++11 -L../dcpots/lib -ldcbase -I../ -static -lprotobuf -ldl -pthread -g3

clean:
	rm -f pbdcexer
	rm -f libpbcex.a
	rm -f *.o

htest: test.cpp ./generater/pbdcex.core.hpp
	g++ test.cpp -g3 -I./generater test.pb.cc extensions.pb.cc -o htest -static -lprotobuf -pthread --std=c++11


libpbcex.a: pbdcex.cpp meta/*.cpp meta/*.cc generater/*.cpp generater/*.h
	g++ -c pbdcex.cpp -std=c++11 -I../ -Igenerater -Imeta -g -O2
	g++ -c generater/*.cpp -Igenerater -Imeta -I../ -std=c++11 -g -O2
	g++ -c meta/*.cpp -Imeta -std=c++11 -g -O2
	g++ -c meta/*.cc -Imeta -std=c++11 -g -O2
	ar r libpbcex.a *.o
	rm -f *.o

install: libpbcex.a htest pbdcexer
	mkdir -p bin
	mkdir -p lib
	mkdir -p include
	mv libpbcex.a lib/
	mv htest bin/
	mv pbdcexer bin/
	cp -f generater/pbdcex.core.hpp include/
	cp -f extensions.proto include/
	cp -f extensions.pb.h include/

