clean:
	rm -f htest

htest: test.cpp proto/test.pb.h proto/test.cex.hpp
	g++ test.cpp -g3 -I../include  -Iproto test.pb.cc extensions.pb.cc -o htest -static -lprotobuf -pthread --std=c++11

proto/test.pb.h: proto/test.protoa ../include/extensions.proto
	protoc test.proto extensions.proto -I../include -Iproto --cpp_out=proto

proto/test.cex.hpp: proto/test.proto
	../bin/pbdcexer -ptest.proto -mHello --cpp_out=proto  -I../include -Iproto
