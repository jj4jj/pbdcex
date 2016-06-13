#include "mysql_gen.h"
#include "mysql/mysql.h"
#include "google/protobuf/compiler/importer.h"
#include <algorithm>
#include <functional>

using namespace std;
using namespace google::protobuf;

///////////////////////////////////////////////////////////////////////////////////////////

MySQLMsgCvt::MySQLMsgCvt(const string & file, void * mysqlconn, size_t MAX_FIELD_BUFFER) :meta_file(file), mysql(mysqlconn){
	field_buffer.reserve(MAX_FIELD_BUFFER);//1MB
	escaped_buffer.reserve(MAX_FIELD_BUFFER * 2 + 1);
}
#define TABLE_NAME_POSTFIX		("_")
#define TABLE_REPEATED_FIELD_POSTFIX	("$")
int		MySQLMsgCvt::CheckMsgValid(const google::protobuf::Descriptor * msg_desc, bool root, bool flatmode ){
	EXTMessageMeta meta;
	if (meta.AttachDesc(msg_desc)){
		//error parse from desc
		cerr << "error parse from desc " << endl;
		return -1;
	}
	if (root){
		if (msg_desc->full_name().find(TABLE_NAME_POSTFIX) != string::npos){
			cerr << "forbidan the db msg type name include :" << TABLE_NAME_POSTFIX << " type:" <<
				msg_desc->full_name() << endl;
			return -2;
		}
		if (meta.pks_name.empty()){
			//not found the pk def
			cerr << "not found the pk def " << endl;
			return -2;
		}
		if (meta.pks_name.size() != meta.pks_fields.size()){
			cerr << "meta pks size not match " << endl;
			return -3;
		}
	}
	//check field type
	for (auto & sfm : meta.sub_fields){
		if (flatmode){
			if (sfm.field_desc->is_repeated() && sfm.z_count <= 0){
				cerr << "not db repeat field but no count define! error field :" << sfm.field_desc->name() << " count:" << sfm.f_count << endl;
				return -4;
			}
			if (sfm.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
				int ret = CheckMsgValid(sfm.field_desc->message_type(), false, true);
				if (ret){
					return -5;
				}
			}
		}
		else {
			if (sfm.field_desc->is_repeated()){
				cerr << "found a repeat field in not a flatmode :" << sfm.field_desc->name() << endl;
				return -6;
			}
		}
	}
	if (root){
		if (meta.m_divnum < 0){
			//must be a integer 
			cerr << "meta divnum is error :" << meta.m_divnum << endl;
			return -7;
		}
		if (!meta.m_divkey.empty() && !msg_desc->FindFieldByName(meta.m_divkey)){
			cerr << "msg :" << msg_desc->name() << " div key:" << meta.m_divkey << " not found !" << endl;
			return -8;
		}
	}
	return 0;
}
int		MySQLMsgCvt::InitMeta(int n , const char ** path, int m , const char ** otherfiles  ){
    if (protometa.Init(path, n, otherfiles, m)){
		cerr << "proto meta init error !" << endl;
		return -1;
	}
    auto filedesc = protometa.LoadFile(meta_file.c_str());
    if (!filedesc){
		cerr << "proto meta load error !" << endl;
		return -2;
	}
    package_name = filedesc->package();
    return 0;
}
string		MySQLMsgCvt::GetTableName(const char * msg_type, int idx){
	string tbname = msg_type;
	if (idx >= 0){
		tbname += TABLE_NAME_POSTFIX;
		tbname += to_string(idx);
	}
	return tbname;
}
string			MySQLMsgCvt::GetRepeatedFieldLengthName(const std::string & name){
	string str_field_name = name;
	str_field_name += TABLE_REPEATED_FIELD_POSTFIX;
	str_field_name += "count";
	return str_field_name;
}
string			MySQLMsgCvt::GetRepeatedFieldName(const std::string & name, int idx){
	string str_field_name = name;
	str_field_name += TABLE_REPEATED_FIELD_POSTFIX;
	str_field_name += to_string(idx);
	return str_field_name;
}
bool		MySQLMsgCvt::IsRepeatedFieldLength(const std::string & field_name, const std::string & key){
	return (key == GetRepeatedFieldLengthName(field_name));
}
int			MySQLMsgCvt::GetMsgFlatTableSQLFields(const google::protobuf::Descriptor * msg_desc, std::string & sql, const std::string & prefix){
	EXTMessageMeta	meta;
	if (meta.AttachDesc(msg_desc)){
		return -1;
	}
	for (auto & field : meta.sub_fields){
		//sql += "`" + GetRepeatedFieldLengthName(field.field_desc->name()) + "` ";
		//sql += "INT UNSIGNED NOT NULL";
		if (field.field_desc->is_repeated()){
			//is repeated , msg or not , count is first
			sql += "`" + prefix + GetRepeatedFieldLengthName(field.field_desc->name()) + "` ";
			sql += "INT UNSIGNED NOT NULL";
			sql += ",\n";		
			for (int i = 0; i < field.z_count; ++i){
				//prefix
				string field_prefix = prefix + GetRepeatedFieldName(field.field_desc->name(), i);
				if (field.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
					if (GetMsgFlatTableSQLFields(field.field_desc->message_type(), sql,
						field_prefix + TABLE_REPEATED_FIELD_POSTFIX)){
						return -2;
					}
				}
				else {
					sql += "`" + field_prefix + "` " + field.GetMysqlFieldType();
					sql += " NOT NULL";
					sql += ",\n";
				}
			}
		}
		else if (field.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
			//unflod
			if (GetMsgFlatTableSQLFields(field.field_desc->message_type(), sql,
				prefix + field.field_desc->name() + TABLE_REPEATED_FIELD_POSTFIX)){
				return -3;
			}
		}
		else {
			sql += "`" + prefix + field.field_desc->name() + "` " + field.GetMysqlFieldType();
			sql += " NOT NULL";
			sql += ",\n";
		}
	}
	return 0;
}

int			MySQLMsgCvt::CreateFlatTables(const char * msg_type, std::string & sql, int idx){
	auto msg_desc = protometa.GetMsgDesc(msg_type);
	if (!msg_desc){
		return -1;
	}
	EXTMessageMeta	meta;
	if (meta.AttachDesc(msg_desc)){
		return -2;
	}
	//template
	string table_name = GetTableName(msg_type, idx);
	sql = "CREATE TABLE IF NOT EXISTS `" + table_name + "` (";
	//append table sql fields type defines	
	GetMsgFlatTableSQLFields(msg_desc, sql, "");

	string pks = "";
	for (auto & pk : meta.pks_name)
	{
		if (!pks.empty()){
			pks += ",";
		}
		pks += "`";
		pks += pk;
		pks += "`";
	}
	sql += "PRIMARY KEY (";
	sql += pks + ")\n";
	sql += ") ENGINE=InnoDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";
	string sql_real = "";
	if (idx < 0 && meta.m_divnum > 0){
		for (int i = 0; i < meta.m_divnum; ++i){
			if (i != 0){
				sql_real += "\n";
			}
			auto new_table_name = GetTableName(msg_type, i);
			sql_real += sql.replace(sql.find(table_name), table_name.length(), new_table_name);
			table_name = new_table_name;
		}
		sql = sql_real;
	}
	sql += ";";
	return 0;
}

int			MySQLMsgCvt::CreateTables(const char * msg_type, std::string & sql,int idx ){
    string msg_type_name = msg_type;
    if (!package_name.empty()){
        msg_type_name = package_name + ".";
        msg_type_name += msg_type;
    }
    auto msg_desc = protometa.GetMsgDesc(msg_type_name.c_str());
	if (!msg_desc){
		return -1;
	}
	EXTMessageMeta	meta;
	if (meta.AttachDesc(msg_desc)){
		return -2;
	}
	//template
	string table_name = GetTableName(msg_type, idx);
	sql = "CREATE TABLE IF NOT EXISTS `" + table_name + "` (";
	for (auto & field : meta.sub_fields)
	{
		sql += "`" + field.field_desc->name() + "` " + field.GetMysqlFieldType();
		sql += " NOT NULL";
		sql += ",\n";
	}
	string pks = "";
	for (auto & pk : meta.pks_name)
	{
		if (!pks.empty()){
			pks += ",";
		}
		pks += "`";
		pks += pk;
		pks += "`";
	}
	sql += "PRIMARY KEY (";
	sql += pks + ")\n";
	sql += ") ENGINE=InnoDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";
	string sql_real = "";
	if (idx < 0 && meta.m_divnum > 0){
		for (int i = 0; i < meta.m_divnum; ++i){
			if (i != 0){
				sql_real += "\n";
			}
			auto new_table_name = GetTableName(msg_type, i);
			sql_real += sql.replace(sql.find(table_name), table_name.length(), new_table_name);
			table_name = new_table_name;
		}
		sql = sql_real;
	}
	sql += ";";
	return 0;
}

int			MySQLMsgCvt::CreateDB(const char * db_name, std::string & sql){
	sql = "CREATE DATABASE IF NOT EXISTS `";
	sql += db_name;
	sql += "` DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";
	return 0;
}
int			MySQLMsgCvt::DropDB(const char * db_name, std::string & sql){
	sql = "DROP DATABASE IF EXISTS `";
	sql += db_name;
	sql += "`;";
	return 0;
}
