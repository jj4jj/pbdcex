clean:
	rm -f htest

htest: test.cpp proto/test.pb.h proto/test.cex.hpp
	g++ test.cpp -g3 -I. -I../include  -Iproto proto/test.pb.cc proto/extensions.pb.cc ${protoi} -o htest ${protol} -pthread --std=c++11

proto/test.pb.h: proto/test.proto ../include/extensions.proto
	${protoc} $^ -Iproto -I../include -I/usr/local/include ${protoi} --cpp_out=proto

proto/test.cex.hpp: proto/test.proto
	../bin/pbdcexer -pproto/test.proto -mHello --cpp_out=. ${protoi} -I../include -Iproto
	#../bin/pbdcexer -pproto/test.proto -mHello --cpp_out=. ${protoi} -I../include -Iproto -t./pbdcex.core.hxx


sql:
	../bin/pbdcexer -pproto/test.proto -mDBPlayer -mDBHello --sql_out=. -I/usr/local/include -I../include -Iproto --sql-flat
	../bin/pbdcexer -pproto/test.proto -mHello --sql_out=. -I/usr/local/include -I../include -Iproto
	../bin/pbdcexer -pproto/test.proto -mDBPlayer --sql_out=. -I/usr/local/include -I../include -Iproto
