#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/descriptor.h"
#include "dcpots/base/logger.h"
#include "extensions.pb.h"
#include "ext_meta.h"

using namespace std;
using namespace google::protobuf;

#define DIM_ARRAY(a)	(sizeof((a))/sizeof((a)[0]))

static inline  void
_strreplace(::std::string & ts, const ::std::string & p, const ::std::string & v) {
	auto beg = 0;
	auto fnd = ts.find(p, beg);
	while (fnd != ::std::string::npos) {
		ts.replace(fnd, p.length(), v);
		beg = fnd + v.length();
		fnd = ts.find(p, beg);
	}
}

#define  _get_option(d,name) (d->options().GetExtension(name))
template<class DescT>
static int64_t			_get_enum_value(const char * name_or_number, DescT desc_) {
	if (!name_or_number || !name_or_number[0]) {
		return 0;
	}
	const char * p = name_or_number;
	long long v = strtoll(name_or_number, (char **)&p, 10);
	if (p != name_or_number) {
		return v;
	}
	auto f_desc = desc_->file();
	auto ev_desc = f_desc->FindEnumValueByName(name_or_number);
	if (ev_desc) {
		return ev_desc->number();
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int	    EXTEnumValueMeta::AttachDesc(const EnumValueDescriptor * desc){
	this->cname=_get_option(desc,::cname);
	this->ev_desc = desc;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////
int	    EXTFieldMeta::AttachDesc(const FieldDescriptor * desc){
	this->alias = _get_option(desc,::alias);
	this->desc = _get_option(desc,::desc);
	this->count = _get_option(desc,::count);
	this->length = _get_option(desc,::length);
	this->ctype = _get_option(desc,::ctype);
	//////////////////////////////////////////////////////////////
	this->vlength = _get_enum_value(length.c_str(), field_desc);
	this->vcount = _get_enum_value(count.c_str(), field_desc);
	if (desc->is_repeated()){
		//cout > 0
		if (this->vcount <= 0){
			GLOG_ERR( "the field: <%s> is a repeated  , but not found options [f_count:%d] or count is a error num value:%d",
				desc->name().c_str(),count.c_str(), vcount);
			return -1;
		}
	}
	if (desc->cpp_type() == FieldDescriptor::CPPTYPE_STRING){
		if (this->vlength <= 0){
			GLOG_ERR("the field: <%s> is a string , but not found options [f_length:%s] or length is a error num value:%d",
				desc->name().c_str(), this->length.c_str(), this->vlength);
			return -2;
		}
	}
	this->field_desc = desc;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
static inline int     _parse_sub_fields(EXTMessageMeta * em) {
	int ret = 0;
	for (int i = 0; i < em->msg_desc->field_count(); ++i) {
		EXTFieldMeta sfm;
		ret = sfm.AttachDesc(em->msg_desc->field(i));
		if (ret) {
			GLOG_ERR("parse field errro in message %s:", em->msg_desc->full_name().c_str());
			return ret;
		}
		em->sub_fields.push_back(sfm);
	}
	return ret;
}
static inline int		_parse_keys_names(EXTMessageMeta * em) {
	string::size_type bpos = 0, fpos = 0;
	while (!em->keys.empty()) {
		fpos = em->keys.find(',', bpos);
		string f_field_name = "";
		if (fpos != string::npos && fpos > bpos) {
			f_field_name = em->keys.substr(bpos, fpos - bpos);
		}
		else {
			if (fpos != bpos) {
				f_field_name = em->keys.substr(bpos);
			}
		}
		_strreplace(f_field_name, " ", "");
		_strreplace(f_field_name, "\t", "");
		em->keys_names.push_back(f_field_name);
		for (auto & suf : em->sub_fields) {
			if (suf.field_desc->name() == f_field_name) {
				em->keys_fields.push_back(&suf);
			}
		}
		if (fpos == string::npos) {
			break;
		}
		bpos = fpos + 1;
	}
	//all fields
	if (em->keys_fields.empty()) {
		for (auto & suf : em->sub_fields) {
			em->keys_fields.push_back(&suf);
		}
	}
	return 0;
}
int	    EXTMessageMeta::AttachDesc(const Descriptor * desc){
	this->keys = _get_option(desc,::keys);
	this->divkey = _get_option(desc,::divkey);
	this->divnum = _get_option(desc,::divnum);
	this->note = _get_option(desc,::note);
	this->relchk = _get_option(desc,::relchk);
	this->autoinc = _get_option(desc,::autoinc);
	this->msg_desc = desc;
	////////////////////////////
	int ret = _parse_sub_fields(this);
    if(ret){
        return ret;
    }
	ret = _parse_keys_names(this);
	//////////////////////////////////////////////////////////////////////////
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////

struct ProtoMetaErrorCollector : google::protobuf::compiler::MultiFileErrorCollector {
	void AddError(
	const string & filename,
	int line,
	int column,
	const string & message) {
		std::cerr << "file name:" << filename << ":" << line << " error:" << message << endl;
	}
};

EXTProtoMeta::EXTProtoMeta(){
    dst = new google::protobuf::compiler::DiskSourceTree();
	dst->MapPath("", ".");
	importer = nullptr;
	dyn_msg_factory = nullptr;
}
EXTProtoMeta::~EXTProtoMeta(){
	if (importer){
		delete importer;
	}
	if (dst){
		delete dst;
	}
	if (dyn_msg_factory){
		delete dyn_msg_factory;
	}
}
int		EXTProtoMeta::Init(const char * * path, int n, const char ** otherfiles , int m ){
	if (importer){
		return -1;
	}
	while (path && n-- > 0){
		dst->MapPath("", path[n]);
		//std::clog << "add path:" << path[n] << endl;
	}
    //dst->MapPath("", "/usr/include");
    //dst->MapPath("", "/usr/local/include");

	static ProtoMetaErrorCollector mfec;
	importer = new google::protobuf::compiler::Importer(dst, &mfec);
#if GOOGLE_PROTOBUF_VERSION >= 2006000
    while (otherfiles && m-- > 0){
        importer->AddUnusedImportTrackFile(otherfiles[m]);
    }
#else
	m = 0;
	otherfiles = NULL;
#endif
	return 0;
}
const google::protobuf::FileDescriptor * EXTProtoMeta::LoadFile(const char * file){
    const FileDescriptor * desc = importer->Import(file);
    if (!desc){
		cerr << "error import file:" << file << endl;
		return nullptr;
	}
	dyn_msg_factory = new DynamicMessageFactory(importer->pool());
	return desc;
}
const google::protobuf::DescriptorPool * EXTProtoMeta::GetPool(){
	return importer->pool();
}
const google::protobuf::Descriptor *	 EXTProtoMeta::GetMsgDesc(const char* msg_type){
	return importer->pool()->FindMessageTypeByName(msg_type);
}
google::protobuf::Message	*			 EXTProtoMeta::NewDynMessage(const char * msg_type, const char * buffer, size_t buffer_len){
	auto msg_desc = GetMsgDesc(msg_type);
	if (!msg_desc){
		cerr << "get msg desc error ! msg_type:" << msg_type << endl;
		return nullptr;
	}
	auto pProtMsg = dyn_msg_factory->GetPrototype(msg_desc);
	if (!pProtMsg){
		cerr << "get proto msg error ! msg_type:" << msg_type << endl;
		return nullptr;
	}
	Message * pMsg = pProtMsg->New();
	if (!pMsg){
		cerr << "new msg from proto error ! msg_type:" << msg_type << endl;
		return nullptr;
	}
	if (buffer_len == 0 || !buffer){
		return pMsg;
	}
	if (!pMsg->ParseFromArray(buffer, (int)buffer_len)){
		cerr << "unpack msg error !" << msg_type << endl;
		FreeDynMessage(pMsg);
		return nullptr;
	}
	return pMsg;
}
void									 EXTProtoMeta::FreeDynMessage(google::protobuf::Message * pMsg){
	delete pMsg;
}
