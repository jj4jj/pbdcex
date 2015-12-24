# pbdcex
protobuf data convert with extensions convert  protobuf message with C/C++ POD struct ,SQL (mysql) each other

##app##
pbdconverter

convert proto message to cpp flat structure (pod) , and create tables .



##lib##
header file: pbdcex.h 
lib: libpbdcex.a
runtime converting 

##make && run##
```sh
  make
  ./pbdconverter -h
  usage: ./pbdconverter [options]	
  option should be like as follow:
  -h, --help           	show the help info
  --cpp_out <arg>      	generate cpp headers dir
  --sql_out <arg>      	generate mysql sql create table headers dir
  -I, --include <arg>  	protobuf include path
  -p, --proto <arg>    	protobuf include proto file
  -m, --message <arg>  	convert messsage full type name

```
