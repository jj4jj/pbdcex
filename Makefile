
pbdcexer: pbdcexer.cpp libpbcex.a ../cxxtemplates/libxctmp.a
	g++ -o $@ $^ -std=c++11 -L../dcpots/lib -ldcbase -I../ -lprotobuf -ldl -g3

clean:
	rm -f pbdcexer
	rm -f libpbcex.a
	rm -f *.o

libpbcex.a: pbdcex.cpp meta/*.cpp meta/*.cc generater/*.cpp generater/*.h
	g++ -c pbdcex.cpp -std=c++11 -I../ -Igenerater -Imeta -g -O2
	g++ -c generater/*.cpp -Igenerater -Imeta -I../ -std=c++11 -g -O2
	g++ -c meta/*.cpp -Imeta -std=c++11 -g -O2
	g++ -c meta/*.cc -Imeta -std=c++11 -g -O2
	ar r libpbcex.a *.o
	rm -f *.o
