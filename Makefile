
pbdconverter: pbdconverter.cpp pbdcex.cpp
	g++ -o $@ $^ -std=c++11 -Ldcpots/lib -ldcbase -Idcpots -Igenerater -Imeta  meta/*.cpp meta/*.cc generater/*.cpp -lprotobuf
