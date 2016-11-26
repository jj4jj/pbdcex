#pragma  once
#include <string>
#include <vector>
namespace google {
	namespace protobuf {
		class Descriptor;
		class FieldDescriptor;
		class FileDescriptor;
		class Reflection;
		class DescriptorPool;
		class Message;
		class EnumValueDescriptor;
		class EnumDescriptor;
		class DynamicMessageFactory;
		namespace compiler {
			class DiskSourceTree;
			class Importer;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
struct EXTEnumValueMeta {
	const google::protobuf::EnumValueDescriptor * ev_desc;
	std::string cname;
	int			AttachDesc(const google::protobuf::EnumValueDescriptor * fd);
};

//////////////////////////////////////////////////////////////////////////////////////////////
struct EXTEnumMeta {
	const google::protobuf::EnumDescriptor * enm_desc;
};


//////////////////////////////////////////////////////////////////////////////////////////////
struct EXTFieldMeta {
	const google::protobuf::FieldDescriptor * field_desc;
	std::string count;
	std::string length;
	std::string alias;
	std::string desc;
	std::string ctype;
	//////////////////////////////////
	int32_t		vlength;
	int32_t		vcount;

public:
	int			AttachDesc(const google::protobuf::FieldDescriptor * fd);

public:
	static int64_t	GetEnumValue(const char * name_or_number, const ::google::protobuf::FieldDescriptor * desc_);

};

struct EXTMessageMeta {
	const google::protobuf::Descriptor * msg_desc;
	std::string keys;
	std::string divkey;
	std::string note;
	std::string relchk;
	std::string autoinc;
	int32_t		divnum{0};
	/////////////////////////////////////////////////////////////
	std::vector<EXTFieldMeta>		sub_fields;
	std::vector<std::string>		keys_names;
	std::vector<EXTFieldMeta*>		keys_fields;//if no , then all
public:
	int	    AttachDesc(const google::protobuf::Descriptor * desc);
};

class EXTProtoMeta {
	google::protobuf::compiler::DiskSourceTree * dst;
	google::protobuf::compiler::Importer *		 importer;
	google::protobuf::DynamicMessageFactory	*	 dyn_msg_factory;
public:
	EXTProtoMeta();
	~EXTProtoMeta();
    int		Init(const char ** paths, int n = 0, const char ** otherfiles = nullptr, int m = 0);
	const google::protobuf::FileDescriptor * LoadFile(const char * file);
	const google::protobuf::DescriptorPool * GetPool();
	const google::protobuf::Descriptor *	 GetMsgDesc(const char * msg_type);
	google::protobuf::Message	*			 NewDynMessage(const char * msg_type, const char * buffer = 0, size_t buffer_len = 0);	
	void									 FreeDynMessage(google::protobuf::Message * msg);

};
