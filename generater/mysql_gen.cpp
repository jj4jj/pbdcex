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
int		MySQLMsgCvt::SetMsgFieldMySQLValue(google::protobuf::Message & msg, const std::string & key, const char * value, size_t value_length){
	const Reflection * pReflection = msg.GetReflection();
	auto msg_desc = msg.GetDescriptor();
	const FieldDescriptor * pField = msg_desc->FindFieldByName(key);
	if (!pField){
		cerr << "not found the msg field from mysql field name :" << key << endl;
		return -1; //not found
	}
	if (pField->is_repeated()){
		//not support
		cerr << "not support repeat field in top layer unfolding mode ! field:" << pField->name() << endl;
		return -1;
	}
	/////////////////////////not repeat . scalar ////////////////////////////////////////
	switch (pField->cpp_type())
	{
	case FieldDescriptor::CPPTYPE_FLOAT:
		pReflection->SetFloat(&msg, pField, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_DOUBLE:
		pReflection->SetDouble(&msg, pField, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT32:
		pReflection->SetInt32(&msg, pField, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT64:
		pReflection->SetInt64(&msg, pField, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT32:
		pReflection->SetUInt32(&msg, pField, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT64:
		pReflection->SetUInt64(&msg, pField, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_ENUM:
		do {
			auto evdesc = pField->enum_type()->FindValueByNumber(atoi(value));
			if (evdesc){
				pReflection->SetEnum(&msg, pField, evdesc);
			}
			else {
				cerr << "not found the enum value:" << value << "! field name:" << pField->name() << " msg type:" << msg_desc->name() << endl;
				return -1;
			}
		} while (false);
		return 0;
	case FieldDescriptor::CPPTYPE_BOOL:
		pReflection->SetBool(&msg, pField, atoi(value) != 0 ? true : false);
		return 0;
	case FieldDescriptor::CPPTYPE_STRING:
		pReflection->SetString(&msg, pField, std::string(value, value_length));
		return 0;
	case FieldDescriptor::CPPTYPE_MESSAGE:
		do{
			auto pSubMsg = pReflection->MutableMessage(&msg, pField);
			if (pSubMsg){
				if (!pSubMsg->ParseFromArray(value, value_length)){
					cerr << "parse mysql field error ! field name:" << pField->name() << " msg type:" << msg_desc->name() << endl;
					return -2;
				}
			}
			else {
				cerr << "mutable sub message error ! field name:" << pField->name() << " msg type:"<< msg_desc->name() << endl;
				return -3;
			}

		} while (false);
		return 0;
	default:
		return -100;
	}
	cerr << "not found field in meta desc key:" << key << " msg type:" << msg_desc->name() << endl;
	return 0;
}
string	MySQLMsgCvt::GetMsgFieldValue(const google::protobuf::Message & msg, const char * key){
	const Reflection * pReflection = msg.GetReflection();
	auto msg_desc = msg.GetDescriptor();
	string lower_case_name = key;
	std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), ::tolower);
	const FieldDescriptor * pField = msg_desc->FindFieldByLowercaseName(lower_case_name);
	if (!pField || pField->is_repeated()){
		cerr << "not found field (or is it a repeat field ? get value not support repeat field) :" << key << " lower case name:" << lower_case_name << endl;
		return "";
	}
	switch (pField->cpp_type())
	{
	case FieldDescriptor::CPPTYPE_FLOAT:
		return std::to_string(pReflection->GetFloat(msg, pField));
	case FieldDescriptor::CPPTYPE_DOUBLE:
		return std::to_string(pReflection->GetDouble(msg, pField));
	case FieldDescriptor::CPPTYPE_INT32:
		return std::to_string(pReflection->GetInt32(msg, pField));
	case FieldDescriptor::CPPTYPE_INT64:
		return std::to_string(pReflection->GetInt64(msg, pField));
	case FieldDescriptor::CPPTYPE_UINT32:
		return std::to_string(pReflection->GetUInt32(msg, pField));
	case FieldDescriptor::CPPTYPE_UINT64:
		return std::to_string(pReflection->GetUInt64(msg, pField));
	case FieldDescriptor::CPPTYPE_ENUM:
		return std::to_string(pReflection->GetEnum(msg, pField)->number());
	case FieldDescriptor::CPPTYPE_BOOL:
		return std::to_string(pReflection->GetBool(msg, pField));
	case FieldDescriptor::CPPTYPE_STRING:
		return pReflection->GetString(msg, pField);
	case FieldDescriptor::CPPTYPE_MESSAGE:
	default:
		cerr << "get value not support repeat field or message field:" << lower_case_name << " type:" << pField->type_name() << endl;
		return "";
	}
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
std::string		MySQLMsgCvt::GetMsgTypeNameFromTableName(const std::string & table_name){
	string::size_type deli = table_name.find_last_of(TABLE_NAME_POSTFIX);
	string msg_type_name;
	if (!package_name.empty()){
		msg_type_name += package_name;
		msg_type_name += ".";
	}
	if (deli == string::npos){
		return msg_type_name + table_name;
	}
	else {
		return msg_type_name + table_name.substr(0, deli);
	}
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
int			MySQLMsgCvt::GetRepeatedFieldIdx(const std::string & field_name, const std::string & key){
	string::size_type fpos = key.find(field_name);
	if (fpos == string::npos){
		return -1;
	}
	fpos += field_name.length();
	if (key.substr(fpos, strlen(TABLE_REPEATED_FIELD_POSTFIX)) != TABLE_REPEATED_FIELD_POSTFIX){
		return -2;
	}
	fpos += strlen(TABLE_REPEATED_FIELD_POSTFIX);
	string sidx = key.substr(fpos);
	if (sidx.empty() || sidx[0] < '0' || sidx[0] > '9'){
		return -3;
	}
	return stoi(sidx);
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
int		MySQLMsgCvt::SetMsgSQLFlatKVRepeated(google::protobuf::Message & msg, 
	const google::protobuf::Reflection * pReflection, 
	const google::protobuf::FieldDescriptor * pField, int idx, const std::string & key, const char * value, size_t value_length){
	for (int i = pReflection->FieldSize(msg, pField); i <= idx; ++i){
		switch (pField->cpp_type())
		{
		case FieldDescriptor::CPPTYPE_FLOAT:
			pReflection->AddFloat(&msg, pField, 0);
			break;
		case FieldDescriptor::CPPTYPE_DOUBLE:
			pReflection->AddDouble(&msg, pField, 0);
			break;
		case FieldDescriptor::CPPTYPE_INT32:
			pReflection->AddInt32(&msg, pField, 0);
			break;
		case FieldDescriptor::CPPTYPE_INT64:
			pReflection->AddInt64(&msg, pField, 0LL);
			break;
		case FieldDescriptor::CPPTYPE_UINT32:
			pReflection->AddUInt32(&msg, pField, 0U);
			break;
		case FieldDescriptor::CPPTYPE_UINT64:
			pReflection->AddUInt64(&msg, pField, 0ULL);
			break;
		case FieldDescriptor::CPPTYPE_ENUM:
			do {
				auto evdesc = pField->enum_type()->FindValueByNumber(0);
				if (evdesc){
					pReflection->AddEnum(&msg, pField, evdesc);
				}
				else {
					cerr << "not found the enum value:" << value << "! field name:" << pField->name() << " msg type:" << msg.GetTypeName() << endl;
					return -1;
				}
			} while (false);
			break;
		case FieldDescriptor::CPPTYPE_BOOL:
			pReflection->AddBool(&msg, pField, false);
			break;
		case FieldDescriptor::CPPTYPE_STRING:
			pReflection->AddString(&msg, pField, "");
			break;
		case FieldDescriptor::CPPTYPE_MESSAGE:
			pReflection->AddMessage(&msg, pField);
			break;
		default:
			return -100;
		}
	}
	switch (pField->cpp_type())
	{
	case FieldDescriptor::CPPTYPE_FLOAT:
		pReflection->SetRepeatedFloat(&msg, pField, idx, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_DOUBLE:
		pReflection->SetRepeatedDouble(&msg, pField, idx, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT32:
		pReflection->SetRepeatedInt32(&msg, pField, idx, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT64:
		pReflection->SetRepeatedInt64(&msg, pField, idx, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT32:
		pReflection->SetRepeatedUInt32(&msg, pField, idx, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT64:
		pReflection->SetRepeatedUInt64(&msg, pField, idx, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_ENUM:
		do {
			auto evdesc = pField->enum_type()->FindValueByNumber(atoi(value));
			if (evdesc){
				pReflection->SetRepeatedEnum(&msg, pField, idx, evdesc);
			}
			else {
				cerr << "not found the enum value:" << value << "! field name:" << pField->name() << " msg type:" << msg.GetTypeName() << endl;
				return -1;
			}
		} while (false);
		return 0;
	case FieldDescriptor::CPPTYPE_BOOL:
		pReflection->SetRepeatedBool(&msg, pField, idx, atoi(value) != 0 ? true : false);
		return 0;
	case FieldDescriptor::CPPTYPE_STRING:
		pReflection->SetRepeatedString(&msg, pField, idx, std::string(value, value_length));
		return 0;
	case FieldDescriptor::CPPTYPE_MESSAGE:
		do{
			auto pSubMsg = pReflection->MutableRepeatedMessage(&msg, pField, idx);
			if (pSubMsg){
				return SetMsgSQLFlatKV(*pSubMsg, key, value, value_length);
			}
			else {
				cerr << "mutable sub message error ! field name:" << pField->name() << " msg type:" << msg.GetTypeName() << endl;
				return -3;
			}

		} while (false);
		return 0;
	default:
		return -100;
	}
}
int		MySQLMsgCvt::SetMsgSQLFlatKV(google::protobuf::Message & msg, const std::string & key, 
	const char * value, size_t value_length){
	const Reflection * pReflection = msg.GetReflection();
	auto msg_desc = msg.GetDescriptor();
	const FieldDescriptor * pField = msg_desc->FindFieldByName(key);
	if (!pField){
		string::size_type field_pos = key.find(TABLE_REPEATED_FIELD_POSTFIX);
		string field_name = key.substr(0, field_pos);
		field_pos += strlen(TABLE_REPEATED_FIELD_POSTFIX); //field_idx_<>
		if (field_name.empty()){
			cerr << "not found the mysql field :" << key << " in msg:" << msg.GetTypeName() << endl;
			return -1;
		}
		pField = msg_desc->FindFieldByName(field_name);
		if (!pField){
			cerr << "not found the mysql field :" << key << " token 1st(field name):" << field_name << " in msg:" << msg.GetTypeName() << endl;
			return -2;
		}
		//-----------------------------------------------------------------------------------------------------------------------
		if (pField->is_repeated()){
			if (IsRepeatedFieldLength(pField->name(), key)){
				//extend size
				return 0; //no  need extend , lazy
			}
			string rep_idx_str = key.substr(field_pos, key.find(TABLE_REPEATED_FIELD_POSTFIX, field_pos));
			field_pos += strlen(TABLE_REPEATED_FIELD_POSTFIX);
			//must be indx
			int rep_idx = stoi(rep_idx_str);
			if (rep_idx >= 0){
				//$idx$v
				field_pos = key.find(TABLE_REPEATED_FIELD_POSTFIX, field_pos);
				field_pos += strlen(TABLE_REPEATED_FIELD_POSTFIX);
				//msg_set(k,v,idx)
				return SetMsgSQLFlatKVRepeated(msg, pReflection, pField, rep_idx, key.substr(field_pos), value, value_length);
			}
			else {
				cerr << "msg (" << msg.GetTypeName() << ")field is repeated , but mysql field name " << key <<  "not match !" << endl;
				return -3;
			}
		}
		else if (pField->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
			auto pSubMsg = pReflection->MutableMessage(&msg, pField);
			if (pSubMsg){
				//msg_set(k,v)
				return SetMsgSQLFlatKV(*pSubMsg, key.substr(field_pos), value, value_length);
			}
			else {
				cerr << "mutable sub message error ! field name:" << pField->name() << " msg type:" << msg_desc->name() << endl;
				return -3;
			}
		}
	}
	/////////////////////////not repeat . scalar ////////////////////////////////////////
	//single
	switch (pField->cpp_type())
	{
	case FieldDescriptor::CPPTYPE_FLOAT:
		pReflection->SetFloat(&msg, pField, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_DOUBLE:
		pReflection->SetDouble(&msg, pField, atof(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT32:
		pReflection->SetInt32(&msg, pField, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_INT64:
		pReflection->SetInt64(&msg, pField, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT32:
		pReflection->SetUInt32(&msg, pField, atoi(value));
		return 0;
	case FieldDescriptor::CPPTYPE_UINT64:
		pReflection->SetUInt64(&msg, pField, atoll(value));
		return 0;
	case FieldDescriptor::CPPTYPE_ENUM:
		do {
			auto evdesc = pField->enum_type()->FindValueByNumber(atoi(value));
			if (evdesc){
				pReflection->SetEnum(&msg, pField, evdesc);
			}
			else {
				cerr << "not found the enum value:" << value << "! field name:" << pField->name() << " msg type:" << msg_desc->name() << endl;
				return -1;
			}
		} while (false);
		return 0;
	case FieldDescriptor::CPPTYPE_BOOL:
		pReflection->SetBool(&msg, pField, atoi(value) != 0 ? true : false);
		return 0;
	case FieldDescriptor::CPPTYPE_STRING:
		pReflection->SetString(&msg, pField, std::string(value, value_length));
		return 0;
	default:
		return -100;
	}
	cerr << "not found field in meta desc key:" << key << " msg type:" << msg_desc->name() << endl;
	return 0;
}
