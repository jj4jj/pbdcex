
pbdconverter: pbdconverter.cpp libpbcex.a
	g++ -o $@ $^ -std=c++11 -L../dcpots/lib -ldcbase -I../ -lprotobuf -ldl

clean:
	rm -f pbdconverter
	rm -f libpbcex.a
	rm -f *.o

libpbcex.a: pbdcex.cpp meta/*.cpp meta/*.cc generater/*.cpp
	g++ -c pbdcex.cpp -std=c++11 -I../ -Igenerater -Imeta -g3
	g++ -c generater/*.cpp -Igenerater -Imeta -std=c++11 -g3
	g++ -c meta/*.cpp -Imeta -std=c++11 -g3
	g++ -c meta/*.cc -Imeta -std=c++11 -g3
	ar r libpbcex.a *.o
	rm -f *.o
