clean:
	rm -f htest

htest: test.cpp proto/test.pb.h test.cex.hpp
	g++ test.cpp -g3 -I. -I../include/pbdcex  -Iproto proto/test.pb.cc proto/extensions.pb.cc ${protoi} -o htest ${protol} -pthread --std=c++11

proto/test.pb.h: proto/test.proto ../include/pbdcex/extensions.proto
	${protoc} $^ -Iproto -I../include/pbdcex -I/usr/local/include ${protoi} --cpp_out=proto

test.cex.hpp: proto/test.proto
	../bin/pbdcexer -pproto/test.proto -mHello --cpp_out=. ${protoi} -I../include/pbdcex -Iproto
	#../bin/pbdcexer -pproto/test.proto -mHello --cpp_out=. ${protoi} -I../include/pbdcex -Iproto -t./pbdcex.core.hxx


sql:
	../bin/pbdcexer -pproto/test.proto -mDBPlayer -mDBHello --sql_out=. -I/usr/local/include -I../include/pbdcex -Iproto --sql-flat
	../bin/pbdcexer -pproto/test.proto -mHello --sql_out=. -I/usr/local/include -I../include/pbdcex -Iproto
	../bin/pbdcexer -pproto/test.proto -mDBPlayer --sql_out=. -I/usr/local/include -I../include/pbdcex -Iproto
