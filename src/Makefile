XCTEMP=../../cxxtemplates
DCPOTS=../../dcpots
INCLUDE=../../
pbdcexer: pbdcexer.cpp libpbdcex.a ${XCTEMP}/lib/libxctmp.a ${DCPOTS}/lib/libdcutil-drs.a  ${DCPOTS}/lib/libpbjson.a ${DCPOTS}/lib/libdcbase.a  
	g++ -o $@ $^ -std=c++11 -I${INCLUDE} ${protoi} -static ${protol} -pthread -ldl -g3

clean:
	rm -f pbdcexer
	rm -f libpbcex.a
	rm -f *.o

libpbdcex.a: pbdcex.cpp meta/ext_meta.cpp  meta/extensions.pb.h ./generater/mysql_gen.cpp ./generater/cpp_gen.cpp
	g++ -c pbdcex.cpp -std=c++11 -Igenerater -Imeta -I${INCLUDE} ${protoi} -g -O2
	g++ -c generater/*.cpp -Igenerater -Imeta -I${INCLUDE}  ${protoi} -std=c++11 -g -O2
	g++ -c meta/*.cpp -Imeta -std=c++11 -I${INCLUDE} ${protoi}  ${protoi} -g -O2
	g++ -c meta/*.cc -Imeta -std=c++11 ${protoi}  ${protoi} -g -O2
	ar r libpbdcex.a *.o
	rm -f *.o

meta/extensions.pb.h: meta/extensions.proto
	${protoc}  $^ --cpp_out=meta ${protoi} -Imeta
