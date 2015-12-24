#include "dcpots/base/stdinc.h"
#include "dcpots/base/cmdline_opt.h"


using namespace std;

int main(int argc, char ** argv){
	//usage
	//covnert --cpp_out=./	--sql_out=./
	//-I path
	//-f file
	//-m message name
	//
	cmdline_opt_t	cmdline(argc, argv);
	cmdline.parse("cpp_out:r::generate cpp headers dir;"
		"sql_out:r::generate mysql sql create table headers dir;"
		"include:r:I:protobuf include path;"
		"proto:r:p:protobuf include proto file;"
		"message:r:m:convert messsage full type name"
		);
	const char * includes_path[16] = { nullptr };
	const char * includes_file[16] = { nullptr };
	const char * messages[64] = { nullptr };
	const char * cpp_out = cmdline.getoptstr("cpp_out");
	const char * sql_out = cmdline.getoptstr("sql_out");
	int	ninc_path = cmdline.getoptnum("include");
	cout << "includes path num:" << ninc_path << endl;
	for (int i = 0; i < ninc_path; ++i){
		includes_path[i] = cmdline.getoptstr("include", i);
		cout << includes_path[i] << endl;
	}
	int	ninc_file = cmdline.getoptnum("proto");
	cout << "includes file num:" << ninc_file << endl;
	for (int i = 0; i < ninc_file; ++i){
		includes_file[i] = cmdline.getoptstr("proto", i);
		cout << includes_file[i] << endl;
	}
	int nmsg = cmdline.getoptnum("message");
	cout << "includes message num:" << nmsg << endl;
	for (int i = 0; i < nmsg; ++i){
		messages[i] = cmdline.getoptstr("message", i);
		cout << messages[i] << endl;
	}


	if (!cpp_out && !sql_out){
		std::cerr << "not found cpp out path and sql out path option " << endl;
		return -1;
	}
	if (ninc_file == 0){
		std::cerr << "not found proto file option" << endl;
		return -2;
	}
	if (nmsg == 0){
		std::cerr << "not found message option" << endl;
		return -3;
	}

	if (cpp_out){
		std::cout << "generating cpp headers to :" << cpp_out << endl;
	}
	if (sql_out){
		std::cout << "generating sql scripts to :" << sql_out << endl;
	}

	return 0;
}



