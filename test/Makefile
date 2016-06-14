clean:
	rm -f htest

htest: test.cpp proto/test.pb.h proto/test.cex.hpp
	g++ test.cpp -g3 -I../include  -Iproto proto/test.pb.cc proto/extensions.pb.cc ${protoi} -o htest -static ${protol} -pthread --std=c++11

proto/test.pb.h: proto/test.proto ../include/extensions.proto
	${protoc} $^ -Iproto -I../include -I/usr/local/include ${protoi} --cpp_out=proto

proto/test.cex.hpp: proto/test.proto
	../bin/pbdcexer -ptest.proto -mHello --cpp_out=proto  -I../include -Iproto
