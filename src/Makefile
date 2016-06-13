XCTEMP=../../cxxtemplates
DCPOTS=../../dcpots
INCLUDE=../../
pbdcexer: pbdcexer.cpp libpbdcex.a ${XCTEMP}/lib/libxctmp.a
	g++ -o $@ $^ -std=c++11 -L${DCPOTS}/lib -ldcbase -I${INCLUDE} -static -lprotobuf -pthread -ldl -g3

clean:
	rm -f pbdcexer
	rm -f libpbcex.a
	rm -f *.o

libpbdcex.a: pbdcex.cpp meta/ext_meta.cpp meta/extensions_option.cpp meta/extensions.pb.cc ./generater/mysql_gen.cpp ./generater/cpp_gen.cpp
	g++ -c pbdcex.cpp -std=c++11 -Igenerater -Imeta -I${INCLUDE} -g -O2
	g++ -c generater/*.cpp -Igenerater -Imeta -I${INCLUDE} -std=c++11 -g -O2
	g++ -c meta/*.cpp -Imeta -std=c++11 -g -O2
	g++ -c meta/*.cc -Imeta -std=c++11 -g -O2
	ar r libpbdcex.a *.o
	rm -f *.o

