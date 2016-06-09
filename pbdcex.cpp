#include "dcpots/base/stdinc.h"
#include "dcpots/utility/drs/dcproto.h"
#include "dcpots/base/logger.h"
#include "google/protobuf/message.h"
#include "google/protobuf/compiler/importer.h"
#include "meta/ext_meta.h"
#include "generater/cpp_gen.h"
#include "generater/mysql_gen.h"

#include "pbdcex.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace dcsutil;
struct pbdcex_t {
	pbdcex_config_t	conf;
	EXTProtoMeta	proto;
	MySQLMsgCvt		* sql_cvt;
	pbdcex_t() :sql_cvt(nullptr){}
};
pbdcex_t *
pbdcex_create(const pbdcex_config_t & conf){
	//check config
	if (conf.meta.files.size() == 0){
		GLOG_ERR("proto files size is 0 ");
		return nullptr;
	}
	pbdcex_t * pdc = new pbdcex_t();
	pdc->conf = conf;
	char * * files = new char *[conf.meta.files.size()];
	for (int i = 0; i < conf.meta.files.size(); ++i){
		files[i] = (char*)conf.meta.files[i].c_str();
	}
	char * * paths = nullptr;
	if (conf.meta.files.size() > 0){
		paths = new char *[conf.meta.paths.size()];
		for (int i = 0; i < conf.meta.paths.size(); ++i){
			paths[i] = (char*)conf.meta.paths[i].c_str();
		}
	}
	if (pdc->proto.Init((const char **)paths, conf.meta.paths.size(), (const char **)files, conf.meta.files.size() - 1)){
		GLOG_ERR("proto meta init erorr !");
		pbdcex_destroy(pdc);
		return nullptr;
	}
	if (!pdc->proto.LoadFile(files[0])){
		GLOG_ERR("proto meta load file:%s erorr !", files[0]);
		pbdcex_destroy(pdc);
		return nullptr;
	}
	pdc->sql_cvt = new MySQLMsgCvt(pdc->conf.meta.files[0], nullptr);
	return pdc;
}
void
pbdcex_destroy(pbdcex_t * pdc){
	if (pdc){
		if (pdc->sql_cvt){
			delete pdc->sql_cvt;
		}
		delete pdc;
	}
}
extern std::stringstream error_stream;

int		
pbdcex_generate_flat_cpp_header(pbdcex_t * pdc, const char * msg){
	auto pool = pdc->proto.GetPool();
	auto desc = pool->FindMessageTypeByName(msg);
	if (!desc){
		GLOG_ERR("not found msg type:%s (should be full name)", msg);
		return -1;
	}
	EXTMessageMeta	smm;
	if (smm.AttachDesc(desc)){
		GLOG_ERR("parse from message desc error :%s !",error_stream.str().c_str());
		return -2;
	}
	error_stream.clear();
	try {
		string file = pdc->conf.cpp.out_path;
		file += "/";			
		file += desc->file()->name();
		if (pdc->conf.cpp.custom_ext_name.empty()){
			file.replace(file.find(".proto"), 6, ".cex.hpp");
		}
		else {
			file.replace(file.find(".proto"), 6, pdc->conf.cpp.custom_ext_name);
		}
		GLOG_IFO("generating file:%s ", file.c_str());
		//generater.DumpFile(file.c_str());
        std::string codegen;
        if (cpph_generate(codegen, desc)){
            GLOG_ERR("generate code error !");
            exit(-1);
        }
        std::ofstream ofs(file, ios::out);
        ofs << codegen ;
		GLOG_IFO("generate code file:%s success !", file.c_str());
	}
	catch (exception & e){
		//GLOG_ERR("generate code error ! for:%s  extra info:", e.what().c_str(), error_stream.str().c_str());
		return -3;
	}
	return 0;
}

int		
pbdcex_generate_mysql_create(pbdcex_t * pdc, const char * msg){
	auto pool = pdc->proto.GetPool();
	auto desc = pool->FindMessageTypeByName(msg);
	if (!desc){
		GLOG_ERR("not found msg type:%s (should be full name)", msg);
		return -1;
	}
	if (pdc->sql_cvt->CheckMsgValid(desc, true, pdc->conf.sql.flat_mode)){
		GLOG_ERR("check msg valid error type:%s !", msg);
		return -2;
	}
	string sql;
	if (pdc->conf.sql.flat_mode){
		if (pdc->sql_cvt->CreateFlatTables(msg, sql)){
			GLOG_ERR("get create flat tables error !");
			return -3;
		}
	}
	else{
		if (pdc->sql_cvt->CreateTables(msg, sql)){
			GLOG_ERR("get create tables error !");
			return -4;
		}
	}
	string outfile = pdc->conf.sql.out_path;
	outfile += "/";
	outfile += msg;
	outfile += ".sql";
	return dcsutil::writefile(outfile, sql.data(), sql.length());
}
int		
pbdcex_generate_mysql_alter(pbdcex_t *, const char * msg, int oldtag){
#warning "todo";
	return -1;
}
