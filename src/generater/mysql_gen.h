#pragma once
#include "../meta/ext_meta.h"
#include <map>

struct MySQLMsgCvtImpl;
class MySQLMsgCvt {
	std::string			meta_file;
	void *			    mysql;
	std::string			field_buffer;
	std::string			escaped_buffer;
    std::string         package_name;
	EXTProtoMeta		protometa; //dynloading
    MySQLMsgCvtImpl     * impl;
public:
	MySQLMsgCvt(const std::string & file, void * mysqlconn, size_t MAX_FIELD_BUFFER = 1024 * 1024);

public:
    int					InitMeta(int n = 0, const char ** path = nullptr, int m = 0, const char ** otherfiles = nullptr);
	int					CheckMsgValid(const google::protobuf::Descriptor * msg_desc, bool root = true, bool flatmode = false);
	int					CreateTables(const char * msg_type, std::string & sql, int idx = -1);
	int					CreateFlatTables(const char * msg_type, std::string & sql, int idx = -1);
	int					CreateDB(const char * db_name, std::string & sql);
	int					DropDB(const char * db_name, std::string & sql);
    int					AlterTables(std::string & sql, std::map<std::string, std::string> old_fields_type);
    std::string			GetTableName(const char * msg_type, int idx = -1);

public:
    std::string		    GetTableName(const google::protobuf::Message * msg);
    int32_t			    GetTableIdx(const google::protobuf::Message * msg);
    int				    Select(std::string & sql, const google::protobuf::Message * msg, std::vector<std::string> * fields = nullptr, 
                            const char * where_ = nullptr, int offset = 0, int limit = 0, const char * orderby = nullptr,
                            int order = 1, bool flatmode = false);
    int				    Delete(std::string & sql, const google::protobuf::Message * msg, const char * where_ = nullptr, bool flatmode = false);
    int				    Replace(std::string & sql, const google::protobuf::Message * msg, bool flatmode = false);
    int				    Update(std::string & sql, const google::protobuf::Message * msg, bool flatmode = false);
    int				    Insert(std::string & sql, const google::protobuf::Message * msg, bool flatmode = false);
    int                 Count(std::string & sql, const google::protobuf::Message * msg, const char * where_ = nullptr);
    int				    CreateTable(std::string & sql, const char * msg_type, bool flatmode = false);
    int				    DropTable(std::string & sql, const char * msg_type);

private:
    static	std::string		GetRepeatedFieldLengthName(const std::string & name);
    static	std::string		GetRepeatedFieldName(const std::string & name, int idx);
    static  bool			IsRepeatedFieldLength(const std::string & field_name, const std::string & key);
    int						GetMsgFlatTableSQLFields(const google::protobuf::Descriptor * msg_desc, std::string & sql, const std::string & prefix);
};
