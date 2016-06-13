#include "dcpots/base/stdinc.h"
#include "dcpots/base/cmdline_opt.h"
#include "pbdcex.h"

using namespace std;

int main(int argc, const char ** argv){
	cmdline_opt_t	cmdline(argc, argv);
	cmdline.parse("cpp_out:r::generate cpp headers dir;"
        "sql_out:r::generate mysql sql create table headers dir;"
        "include:r:I:protobuf include path:.;"
        "proto:r:p:protobuf include proto file;"
		"message:r:m:convert messsage full type name;"
		"template:r:t:a c++ code generator template file"
		);
	const char * includes_path[16] = { nullptr };
	const char * includes_file[16] = { nullptr };
	const char * messages[64] = { nullptr };
	const char * cpp_out = cmdline.getoptstr("cpp_out");
	const char * sql_out = cmdline.getoptstr("sql_out");
	int	ninc_path = cmdline.getoptnum("include");

    cout << "includes paths : ";
	for (int i = 0; i < ninc_path; ++i){
        if (i > 0){
            cout << " , ";
        }
		includes_path[i] = cmdline.getoptstr("include", i);
		cout << includes_path[i];
	}
    cout << endl;
	int	ninc_file = cmdline.getoptnum("proto");
    cout << "includes files : ";
	for (int i = 0; i < ninc_file; ++i){
        if (i > 0){
            cout << " , ";
        }
		includes_file[i] = cmdline.getoptstr("proto", i);
		cout << includes_file[i];
	}
    cout << endl;
	int nmsg = cmdline.getoptnum("message");
	cout << "convert message root : ";
	for (int i = 0; i < nmsg; ++i){
        if (i > 0){
            cout << " , ";
        }
		messages[i] = cmdline.getoptstr("message", i);
		cout << messages[i] << endl;
	}
    pbdcex_config_t config;
    for (int i = 0; i < ninc_path; ++i){
        config.meta.paths.push_back(includes_path[i]);
    }
    for (int i = 0; i < ninc_file; ++i) {
        config.meta.files.push_back(includes_file[i]);
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
        config.cpp.custom_ext_name = ".cex.hpp";
        config.cpp.out_path = cpp_out;
        std::cout << "generating cpp headers to dir [" << cpp_out << "] ..." << endl;
	}
	if (sql_out){
        config.sql.flat_mode = false;
        config.sql.out_path = sql_out;
        //std::cout << "generating sql scripts to :" << sql_out <<" ..."<< endl;
	}

    pbdcex_t * pbdc =  pbdcex_create(config);
    if (!pbdc){
        cerr << "error init" << endl;
        return -1;
    }
    int		ret = pbdcex_generate_flat_cpp_header(pbdc, messages[0], cmdline.getoptstr("template"));
    if(ret){
        std::cerr << "pbdcex convert error ! code : " << ret << std::endl;
    }
    else {
        std::cout << "message generating success !" << endl;
    }
    pbdcex_destroy(pbdc);
	return 0;
}



