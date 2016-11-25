#pragma once
#include <string>
#include <vector>
struct MySQLMsgMetaImpl;
struct MySQLRow;
struct EXTMessageMeta;
namespace google {
	namespace protobuf {
		class Descriptor;
		class Messsage;
	}
}
struct MySQLMsgMeta {
    int					InitMeta(int n = 0, const char ** path = nullptr, int m = 0, const char ** otherfiles = nullptr);
	int					CheckMsgValid(const std::string & name, bool root = true, bool flatmode = false);
	const google::protobuf::Descriptor * 
						GetMsgDescriptor(const std::string & name) const;
	const EXTMessageMeta *
						GetMsgExMeta(const std::string & name);
public:
	//msg
	int					CreateTables(std::string & sql, const std::string & name, int idx = -1, bool flatmode = false);
	int					CreateDB(const std::string & name, std::string & sql);
	int					DropDB(const std::string & name, std::string & sql);

public:
	//msg
	std::string		    GetTableName(const google::protobuf::Message &msg);
    int				    Select(std::string & sql, const google::protobuf::Message & msg_key, std::vector<std::string> * fields = nullptr,
                            const char * where_ = nullptr, int offset = 0, int limit = 0, const char * orderby = nullptr,
                            int order = 1, bool flatmode = false);
    int				    Delete(std::string & sql, const google::protobuf::Message & msg, const char * where_ = nullptr, bool flatmode = false);
    int				    Replace(std::string & sql, const google::protobuf::Message & msg, bool flatmode = false);
    int				    Update(std::string & sql, const google::protobuf::Message & msg, bool flatmode = false);
    int				    Insert(std::string & sql, const google::protobuf::Message & msg, bool flatmode = false);
    int                 Count(std::string & sql, const google::protobuf::Message & msg, const char * where_ = nullptr);
    int				    CreateTable(std::string & sql, const google::protobuf::Message & msg, bool flatmode = false);
    int				    DropTable(std::string & sql, const google::protobuf::Message & msg);

public:
	int					GetMsgFromSQLRow(google::protobuf::Message & msg, const MySQLRow &  row);


public:
	MySQLMsgMeta(const std::string & file, void * mysqlconn, size_t MAX_FIELD_BUFFER = 1024 * 1024);
	~MySQLMsgMeta();
private:
	MySQLMsgMetaImpl     * impl;
};
