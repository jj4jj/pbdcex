#!/bin/bash

protoc="protoc"
protol="-lprotobuf"
protoi="-I/usr/local/include"
if [[ "x$1" = "xu1" ]];then
	protoc='../../protobuf/bin/protoc'
	protoi='-I../../protobuf/include/'
	protol='../../protobuf/lib/libprotobuf.a'
fi
make install protoc="$protoc" protoi="$protoi" protol="$protol"
make test protoc="$protoc" protoi="$protoi" protol="$protol"

