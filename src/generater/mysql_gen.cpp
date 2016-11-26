#include "dcpots/base/stdinc.h"
#include "mysql/mysql.h"
#include "google/protobuf/compiler/importer.h"
#include "../meta/ext_meta.h"
#include <unordered_map>
#include "dcpots/base/logger.h"
#include "dcpots/utility/drs/dcproto.h"


//////////////////////////////////////////////////////////////////////////
#include "mysql_gen.h"
//////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dcs;
using namespace google::protobuf;
///////////////////////////////////////////////////////////////////////////////////////////
struct st_mysql_field;
struct MySQLRow {
	const char *				table_name;
	const char * *				fields_name;
	int							num_fields;
	const char **				row_data;
	unsigned long *				row_lengths;
};
typedef std::unordered_map<std::string, EXTMessageMeta>		MySQLMsgMetaCache;
struct MySQLMsgMetaImpl {
	std::string			meta_file;
	void *			    mysql;
	std::string			field_buffer;
	std::string			escaped_buffer;
	std::string         package_name;
	EXTProtoMeta		protometa; //dynloading
	MySQLMsgMetaCache	msg_meta_cache;
};
#define TABLE_NAME_IDX_SEP		("_")
#define TABLE_FLAT_FIELD_LAYER_SEP		("__")
#define TABLE_FLAT_FIELD_IDX_SEP	("$")
//////////////////////////////////////////////////////////////////////////
MySQLMsgMeta::MySQLMsgMeta(const string & file, void * mysqlconn, size_t MAX_FIELD_BUFFER){
	impl = new MySQLMsgMetaImpl();
	impl->meta_file = file;
	impl->mysql = mysqlconn;
	impl->field_buffer.reserve(MAX_FIELD_BUFFER);//1MB
	impl->escaped_buffer.reserve(MAX_FIELD_BUFFER * 2 + 1);
}
MySQLMsgMeta::~MySQLMsgMeta() {
	if(impl){delete impl; impl = nullptr;}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////implementation
static inline std::string get_mysql_field_type_name(const EXTFieldMeta * em) {
	switch (em->field_desc->cpp_type()) {
	case FieldDescriptor::CPPTYPE_INT32:// 1,     // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
	case FieldDescriptor::CPPTYPE_ENUM:// 8,     // TYPE_ENUM
		return "INT";
	case FieldDescriptor::CPPTYPE_UINT32:// 3,     // TYPE_UINT32, TYPE_FIXED32
		return "INT UNSIGNED";
	case FieldDescriptor::CPPTYPE_INT64:// 2,     // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
		return "BIGINT";
	case FieldDescriptor::CPPTYPE_UINT64:// 4,     // TYPE_UINT64, TYPE_FIXED64
		return "BIGINT UNSIGNED";
	case FieldDescriptor::CPPTYPE_DOUBLE:// 5,     // TYPE_DOUBLE
		return "DOUBLE";
	case FieldDescriptor::CPPTYPE_FLOAT:// 6,     // TYPE_FLOAT
		return "FLOAT";
	case FieldDescriptor::CPPTYPE_BOOL:// 7,     // TYPE_BOOL
		return "TINYINT";
	case FieldDescriptor::CPPTYPE_STRING:// 9,     // TYPE_STRING, TYPE_BYTES
		if (em->vlength <= 0XFF) {
			return "VARCHAR("+to_string(em->vlength)+")"; //255
		}
		else if (em->vlength <= 0XFFFF) {
			return "TEXT";//64K
		}
		else if (em->vlength <= 0XFFFFFF) {
			return "MEDIUMTEXT"; //16MB
		}
		else {
			return "LONGTEXT";//4GB
		}
	case FieldDescriptor::CPPTYPE_MESSAGE:// 10,    // TYPE_MESSAGE, TYPE_GROUP	}
		return "MEDIUMBLOB";	//16MB
		/*
		if (zSize <= 0XFFFF){
		return "BLOB";	//64K
		}
		else if(zSize <= 0XFFFFFF) {
		return "MEDIUMBLOB";	//16MB
		}
		else {
		return "LOGNGBLOB"; //4GB
		}
		*/
	default:
		return "MEDIUMBLOB";
	}
}
const google::protobuf::Descriptor * GetMsgDescriptorImpl(MySQLMsgMetaImpl * impl, const std::string & name) {
	return impl->protometa.GetMsgDesc(name.data());
}
static inline int	CheckMsgValidImpl(MySQLMsgMetaImpl * impl, const std::string & name, bool root, bool flatmode) {
	EXTMessageMeta meta;
	const Descriptor * msg_desc = GetMsgDescriptorImpl(impl, name);
	if (!msg_desc) {
		GLOG_ERR("not found desc by name:%s", name.c_str());
		return -1;
	}
	if (meta.AttachDesc(msg_desc)) {
		//error parse from desc
		GLOG_ERR("error parse from desc name:%s", name.c_str());
		return -1;
	}
	if (root) {
		if (msg_desc->full_name().find(TABLE_NAME_IDX_SEP) != string::npos) {
			GLOG_ERR("forbidan the db msg type name include :%s type:%s", TABLE_NAME_IDX_SEP, msg_desc->full_name().c_str());
			return -2;
		}
		if (meta.keys_names.empty()) {
			//not found the pk def
			GLOG_ERR("not found the primary key declaration of msg:%s!", msg_desc->full_name().c_str());
			return -2;
		}
		if (meta.keys_names.size() != meta.keys_fields.size()) {
			GLOG_ERR("meta primary key list size (%zu,%zu) not match msg:%s !",
				meta.keys_names.size(), meta.keys_fields.size(), msg_desc->full_name().c_str());
			return -3;
		}
	}
	//check field type
	for (auto & sfm : meta.sub_fields) {
		if (flatmode) {
			if (sfm.field_desc->is_repeated() && sfm.vcount <= 0 ) {
				GLOG_ERR("not db repeat field but no count define! error field :%s count:%s",sfm.field_desc->name().c_str(), sfm.count.c_str() );
				return -4;
			}
			if (sfm.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
				int ret = CheckMsgValidImpl(impl, sfm.field_desc->message_type()->name(), false, true);
				if (ret) {
					return -5;
				}
			}
		}
		#if 0
		else {
			if (sfm.field_desc->is_repeated()) {
				GLOG_ERR("found a repeat field in not a flatmode field:%s", sfm.field_desc->name().c_str());
				return -6;
			}
		}
		#endif
	}
	if (root) {
		if (meta.divnum < 0) {
			//must be a integer 
			GLOG_ERR("meta divnum is error :%d", meta.divnum );
			return -7;
		}
		if (!meta.divkey.empty() && !msg_desc->FindFieldByName(meta.divkey)) {
			GLOG_ERR("msg type %s div key:%s not found !" ,msg_desc->name().c_str(), meta.divkey.c_str());
			return -8;
		}
	}
	return 0;
}
static inline const EXTMessageMeta * GetMsgMetaImpl(MySQLMsgMetaImpl * impl, const std::string & name) {
	auto it = impl->msg_meta_cache.find(name);
	if (it == impl->msg_meta_cache.end()) {
		EXTMessageMeta		etm;
		if (etm.AttachDesc(GetMsgDescriptorImpl(impl,name))) {
			return nullptr;
		}
		impl->msg_meta_cache[name] = etm;
		return &impl->msg_meta_cache[name];
	}
	return &it->second;
}
int		MySQLMsgMeta::InitMeta(int n, const char ** path, int m, const char ** otherfiles) {
	if (impl->protometa.Init(path, n, otherfiles, m)) {
		GLOG_ERR("proto meta init error !" );
		return -1;
	}
	auto filedesc = impl->protometa.LoadFile(impl->meta_file.c_str());
	if (!filedesc) {
		GLOG_ERR("proto meta load error !" );
		return -2;
	}
	impl->package_name = filedesc->package();
	return 0;
}
static inline string	GetMsgTableNameImpl(const std::string & name, int idx) {
	string tbname = name;
	if (idx > 0) {
		tbname += TABLE_NAME_IDX_SEP;
		tbname += to_string(idx);
	}
	return tbname;
}
////////////////////////////////////////////////////////////////////////////////////////////////
//////declarations //////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
const google::protobuf::Descriptor *
MySQLMsgMeta::GetMsgDescriptor(const std::string & name) const {
	return GetMsgDescriptorImpl(impl, name);
}
const EXTMessageMeta *
MySQLMsgMeta::GetMsgExMeta(const std::string & name) {
	return GetMsgMetaImpl(impl, name);
}
int		MySQLMsgMeta::CheckMsgValid(const std::string & name, bool root, bool flatmode) {
	return CheckMsgValidImpl(impl, name, root, flatmode);
}
static inline int32_t GetTableIdxImpl(MySQLMsgMetaImpl * impl, const google::protobuf::Message & msg) {
	auto pmeta = GetMsgMetaImpl(impl, msg.GetTypeName());
	if (!pmeta) {
		return -1;
	}
	if (pmeta->divnum > 0) {
		uint64_t ullspkey = stoull(dcs::protobuf_msg_field_get_value(msg, pmeta->divkey, 0).c_str());
		return ullspkey % pmeta->divnum;
	}
	return 0;
}
static inline std::string GetTableNameImpl(MySQLMsgMetaImpl * impl, const string & name, int idx) {
	string tb_name = name;
	if (idx >= 0) {
		tb_name += "_";
		tb_name += to_string(idx);
	}
	return tb_name;
}
static inline int   Escape(MySQLMsgMetaImpl * impl, std::string & result, const char * data, int datalen) {
	if (datalen <= 0) {
		result = "''";
		return 0;
	}
	/////////////////////////////////////////    
	result = "'";
	if (impl->mysql) {
		memset((char*)&impl->escaped_buffer[0], 0, 2 * datalen + 1);
		mysql_real_escape_string((st_mysql*)impl->mysql,
			(char*)&impl->escaped_buffer[0], data, datalen);
	}
	else {
		GLOG_ERR("escaped string but msyql connection is null ! data:%p size:%d", data, datalen);
		result = "''";
		return -1;
	}
	result.append(impl->escaped_buffer.data());
	result.append("'");
	return 0;
}
/////////////////////////////////////////////////////////////////////////
static inline std::string	GetMsgTypeNameFromTableName(MySQLMsgMetaImpl * impl, const std::string & table_name) {
	string::size_type deli = table_name.find_last_of(TABLE_NAME_IDX_SEP);
	string msg_type_name;
	if (!impl->package_name.empty()) {
		msg_type_name += impl->package_name;
		msg_type_name += ".";
	}
	if (deli == string::npos) {
		return msg_type_name + table_name;
	}
	else {
		return msg_type_name + table_name.substr(0, deli);
	}
}

static inline string	GetRepeatedFieldLengthName(const std::string & name){
	string str_field_name = name;
	str_field_name += TABLE_FLAT_FIELD_IDX_SEP;
	str_field_name += "count";
	return str_field_name;
}
static inline string	GetRepeatedFieldName(const std::string & name, int idx){
	string str_field_name = name;
	str_field_name += TABLE_FLAT_FIELD_IDX_SEP;
	str_field_name += to_string(idx);
	return str_field_name;
}
static inline bool	IsRepeatedFieldLength(MySQLMsgMetaImpl * impl, const std::string & field_name, const std::string & key){
	return (key == GetRepeatedFieldLengthName(field_name));
}
static inline int	GetMsgTableSQLFields(MySQLMsgMetaImpl * impl, const std::string & name, std::string & sql,
										 const std::string & prefix, bool flatmode = true){
	auto pmeta = GetMsgMetaImpl(impl, name);
	if (!pmeta) {
		return -1;
	}
	const EXTMessageMeta & meta = *pmeta;
	for (auto & field : meta.sub_fields){
		//sql += "`" + GetRepeatedFieldLengthName(field.field_desc->name()) + "` ";
		//sql += "INT UNSIGNED NOT NULL";
		if (field.field_desc->is_repeated()){
			//is repeated , msg or not , count is first
			sql += "`" + prefix + GetRepeatedFieldLengthName(field.field_desc->name()) + "` ";
			sql += "INT UNSIGNED NOT NULL";
			sql += ",\n";		
			for (int i=0 ; i < field.vcount; ++i){
				//prefix
				string field_prefix = prefix + GetRepeatedFieldName(field.field_desc->name(), i);
				if (flatmode && field.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
					if (GetMsgTableSQLFields(impl, field.field_desc->message_type()->name(), sql,
						field_prefix + TABLE_FLAT_FIELD_LAYER_SEP)){
						return -2;
					}
				}
				else {
					sql += "`" + field_prefix + "` " + get_mysql_field_type_name(&field);
					sql += " NOT NULL";
					sql += ",\n";
				}
			}
		}
		else if (flatmode && field.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
			//unflod
			if (GetMsgTableSQLFields(impl, field.field_desc->message_type()->name(), sql,
				prefix + field.field_desc->name() + TABLE_FLAT_FIELD_LAYER_SEP)){
				return -3;
			}
		}
		else {
			sql += "`" + prefix + field.field_desc->name() + "` " + get_mysql_field_type_name(&field);
			sql += " NOT NULL";
			sql += ",\n";
		}
	}
	return 0;
}
//implementaion
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int			MySQLMsgMeta::CreateTables(std::string & sql, const std::string & name, int idx , bool flatmode){
	auto * pmeta = GetMsgExMeta(name);
	if (!pmeta) {
		return -1;
	}
	const EXTMessageMeta & meta = *pmeta;
	//////////////////////////////////////////////////////////////////////////
	//template
	string table_name = GetTableNameImpl(impl, name, idx);
	sql = "CREATE TABLE IF NOT EXISTS `" + table_name + "` (";
	//append table sql fields type defines	
	GetMsgTableSQLFields(impl, name, sql, "", flatmode);
	string pks = "";
	for (auto & pk : meta.keys_names){
		if (!pks.empty()) {
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
	if (idx < 0 && meta.divnum > 0) {
		for (int i = 0; i < meta.divnum; ++i) {
			if (i != 0) {
				sql_real += "\n";
			}
			auto new_table_name = GetTableNameImpl(impl, name, i);
			sql_real += sql.replace(sql.find(table_name), table_name.length(), new_table_name);
			table_name = new_table_name;
		}
		sql = sql_real;
	}
	sql += ";";
	return 0;
}

int			MySQLMsgMeta::CreateDB(const std::string &  db_name, std::string & sql){
	sql = "CREATE DATABASE IF NOT EXISTS `";
	sql += db_name;
	sql += "` DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";
	return 0;
}
int			MySQLMsgMeta::DropDB(const std::string &  db_name, std::string & sql){
	sql = "DROP DATABASE IF EXISTS `";
	sql += db_name;
	sql += "`;";
	return 0;
}
std::string		MySQLMsgMeta::GetTableName(const google::protobuf::Message & msg) {
	return GetTableNameImpl(impl, msg.GetTypeName(), GetTableIdxImpl(impl, msg));
}
//////////////////////////////////////////////////////////////////////////
typedef std::vector<std::pair<std::string, std::string > >	MsgSQLKVList;
struct fetch_msg_sql_kvlist_ctx {
	MsgSQLKVList  * kvlist{nullptr};
	vector<string>	prefix;
	bool			is_in_array{false};
	std::string		buffer;
	MySQLMsgMetaImpl * impl;
};
static inline const std::string & msyql_field_value_ref_protobuf(MySQLMsgMetaImpl *impl, std::string & result, const std::string & v, const FieldDescriptor * fdsc) {
	int ret = 0;
	switch (fdsc->cpp_type()) {
		case FieldDescriptor::CPPTYPE_INT32 :     // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
		case FieldDescriptor::CPPTYPE_INT64 :     // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
		case FieldDescriptor::CPPTYPE_UINT32 :     // TYPE_UINT32, TYPE_FIXED32
		case FieldDescriptor::CPPTYPE_UINT64 :     // TYPE_UINT64, TYPE_FIXED64
		case FieldDescriptor::CPPTYPE_DOUBLE :     // TYPE_DOUBLE
		case FieldDescriptor::CPPTYPE_FLOAT :     // TYPE_FLOAT
		return v;
		case FieldDescriptor::CPPTYPE_BOOL :     // TYPE_BOOL
			if (v == "true" || v == "1" || v == "yes" || v == "1") {
				result = "1";
			}
			else {
				result = "0";
			}
			return result;
		case FieldDescriptor::CPPTYPE_ENUM:     // TYPE_ENUM
			if (dcs::strisint(v)) {
				return v;
			}
			else {
				auto fxc = fdsc->file();
				auto ev_desc = fxc->FindEnumValueByName(v);
				if (ev_desc) {
					result = to_string(ev_desc->number());
				}
				else {
					GLOG_ERR("not found enum def value:%s for field:%s", v.c_str(), fdsc->full_name().c_str());
					result = "0";
				}
				return result;
			}
		break;
		case FieldDescriptor::CPPTYPE_STRING:     // TYPE_STRING, TYPE_BYTES
		case FieldDescriptor::CPPTYPE_MESSAGE:    // TYPE_MESSAGE, TYPE_GROUP
		ret = Escape(impl, result, v.data(), v.length());
		if(ret){
			GLOG_ERR("escape field:%s error:%d !", fdsc->full_name().c_str(), ret);
		}		
		return result;
		break;
		default:
		return v;
	}
}
static void fetch_msg_sql_kvlist(const string & name, const google::protobuf::Message & msg, int idx, int level, void *ud, protobuf_sax_event_type evt) {
	fetch_msg_sql_kvlist_ctx	* ctx = (fetch_msg_sql_kvlist_ctx*)ud;
	string key;
	switch (evt) {
		case BEGIN_MSG:
			if (ctx->is_in_array) {
				ctx->prefix.push_back(name+ TABLE_FLAT_FIELD_IDX_SEP +to_string(idx));
			}
			else {
				ctx->prefix.push_back(name);
			}
		break;
		case END_MSG:
		ctx->prefix.pop_back();
		break;
		case BEGIN_ARRAY:
		ctx->is_in_array = true;
		break;
		case END_ARRAY:
		ctx->is_in_array = false;
		break;
		case VISIT_VALUE:
			strjoin(key,TABLE_FLAT_FIELD_LAYER_SEP,ctx->prefix);
			if (ctx->is_in_array) {
				key += TABLE_FLAT_FIELD_IDX_SEP + to_string(idx);
			}
			ctx->kvlist->push_back(make_pair(key, 
					msyql_field_value_ref_protobuf(ctx->impl, ctx->buffer, 
					protobuf_msg_field_get_value(msg, name, idx), msg.GetDescriptor()->FindFieldByName(name))));
		break;
	}
}

static inline int GetMsgSQLKVList(MySQLMsgMetaImpl * impl, const Message & msg, MsgSQLKVList & kvlist, bool flatmode) {
	fetch_msg_sql_kvlist_ctx ctx;
	ctx.kvlist = &kvlist;
	ctx.impl = impl;
	if (flatmode) {
		protobuf_msg_sax("", msg, fetch_msg_sql_kvlist, &ctx, 0, false);
	}
	else {
		for (int i = 0;i < msg.GetDescriptor()->field_count(); ++i) {
			auto field = msg.GetDescriptor()->field(i);
			if (field->is_repeated()) {
				int xc = msg.GetReflection()->FieldSize(msg, field);
				if (xc > 0) {
					kvlist.push_back(make_pair(GetRepeatedFieldLengthName(field->name()), std::to_string(xc)));
					for (int x = 0; x < xc; ++x) {
						kvlist.push_back(make_pair(GetRepeatedFieldName(field->name(), x),
							msyql_field_value_ref_protobuf(impl, ctx.buffer,
							protobuf_msg_field_get_value(msg, field->name(), x), field)));
					}
				}
			}
			else if (msg.GetReflection()->HasField(msg, field)) {
				kvlist.push_back(make_pair(field->name(), 
						msyql_field_value_ref_protobuf(impl, ctx.buffer,
						protobuf_msg_field_get_value(msg, field->name(), -1), field)));
			}
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
static inline int SetMsgSQLKVList(Message & msg, const MySQLRow & sql_row) {
	int ret = 0;
	std::string error;
	std::string	value;
	value.reserve(1024 * 4);
	std::string	ppath = "";
	for (int i = 0;i < sql_row.num_fields; ++i) {
		ppath = sql_row.fields_name[i];
		strreplace(ppath, TABLE_FLAT_FIELD_LAYER_SEP, ".");
		strreplace(ppath, TABLE_FLAT_FIELD_IDX_SEP, ":");
		if (ppath.find_last_of(":count") + 6 == (ppath.length())) {
			continue;//count
		}
		value.assign(sql_row.row_data[i], sql_row.row_lengths[i]);
		ret = protobuf_msg_field_path_set_value(msg, ppath, value, error);
		if (ret) {
			GLOG_ERR("msg field  path:%s set error:%d (%s)!", ppath.c_str(), ret, error.c_str());
			return ret;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
int			MySQLMsgMeta::Select(std::string & sql, const google::protobuf::Message & msg_key, std::vector<std::string> * fields ,
							const char * where_ , int offset , int limit , const char * orderby ,
							int order , bool flatmode ) {
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	//
	sql = "SELECT ";
	if (fields && !fields->empty()) {
		for (int i = 0; i < (int)fields->size(); ++i) {
			if (i != 0) {
				sql += ",";
			}
			sql += "`";
			sql += fields->at(i);
			sql += "`";
		}
	}
	else {
		sql += "*";
	}
	sql += " FROM `";
	sql += GetTableName(msg_key);
	sql += "` ";
	if (where_ && where_[0]) {
		sql += " WHERE ";
		sql += where_;
	}
	else {
		sql += " WHERE ";
		int is_first = 0;
		int is_pk = 0;
		MsgSQLKVList kvlist;
		int ret = GetMsgSQLKVList(impl, msg_key, kvlist, flatmode);
		if (ret) {
			GLOG_ERR("error msg sql get kv list!");
			return ret;
		}
		if (kvlist.empty()) {
			sql += " TRUE ";
		}
		else {
			for (auto & kv : kvlist){
				is_pk = 0;
				for (auto & pk : pmeta->keys_names){
					if (kv.first == pk){
						is_pk = true;
						break;
					}
				}
				if (!is_pk){
					continue;
				}
				if (is_first != 0){
					sql += " AND ";
				}
				is_first = 1;
				sql += "`";
				sql += kv.first;
				sql += "`";
				sql += " = ";
				sql += kv.second;
			}
		}
	}
	//ORDER BY todo
	if (orderby && *orderby) {
		sql += " ORDER BY ";
		sql += orderby;
		sql += order > 0 ? " ASC" : " DESC";
	}
	//LIMIT 0,100
	//=LIMIT 100 OFFSET 0
	if (limit < 0) {
		limit = 0;
	}
	sql += " LIMIT ";
	sql += to_string(limit);
	if (offset > 0) {
		sql += " OFFSET ";
		sql += to_string(offset);
	}
	sql += ";";
	return 0;
}
int			MySQLMsgMeta::Delete(std::string & sql, const google::protobuf::Message & msg_key, const char * where_ , bool flatmode ){
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}

	sql = "DELETE ";
	sql += " FROM `";
	sql += GetTableName(msg_key);
	sql += "` WHERE ";
	if (where_) {
		sql += where_;
	}
	else {
		int is_first = 0;
		int is_pk = 0;
		MsgSQLKVList kvlist;
		int ret = GetMsgSQLKVList(impl, msg_key, kvlist, flatmode);
		if (ret) {
			GLOG_ERR("error msg sql get kv list!");
			return ret;
		}
		for (auto & kv : kvlist)
		{
			is_pk = 0;
			for (auto & pk : pmeta->keys_names)
			{
				if (kv.first == pk)
				{
					is_pk = true;
					break;
				}
			}
			if (!is_pk)
			{
				continue;
			}
			if (is_first != 0)
			{
				sql += " AND ";
			}
			is_first = 1;

			sql += kv.first;
			sql += " = ";
			sql += kv.second;
		}
	}
	sql += ";";
	return 0;
}
int			MySQLMsgMeta::Replace(std::string & sql, const google::protobuf::Message & msg_key, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	sql = "REPLACE INTO `";
	sql += GetTableName(msg_key);
	sql += "` (";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(impl, msg_key, kvlist, flatmode);
	if (ret) {
		GLOG_ERR("error msg sql get kv list!");
		return ret;
	}
	for (int i = 0; i < (int)kvlist.size(); ++i)
	{
		if (i != 0)
		{
			sql += ",";
		}
		sql += "`";
		sql += kvlist[i].first;
		sql += "`";
	}
	sql += ") VALUES (";
	//VALUES(vs)
	for (int i = 0; i < (int)kvlist.size(); ++i)
	{
		if (0 != i)
		{
			sql += ",";
		}
		sql += kvlist[i].second;
	}
	sql += ");";
	return 0;
}
int			MySQLMsgMeta::Update(std::string & sql, const google::protobuf::Message & msg_key, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	/*
	MySQL
	UPDATE[LOW_PRIORITY][IGNORE] tbl_name
	SET col_name1 = expr1[, col_name2 = expr2 ...]
	[WHERE where_definition]
	[ORDER BY ...]
	[LIMIT row_count]
	*/
	sql = "UPDATE `";
	sql += GetTableName(msg_key);
	sql += "` SET ";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(impl, msg_key, kvlist, flatmode);
	if (ret) {
		GLOG_ERR("error msg sql get kv list!");
		return ret;
	}

	int is_first = 0;
	int is_pk = 0;
	for (auto & kv : kvlist)
	{
		is_pk = 0;
		for (int j = 0; j < (int)pmeta->keys_names.size(); ++j)
		{
			if (kv.first == pmeta->keys_names[j])
			{
				is_pk = true;
				break;
			}
		}
		if (is_pk || kv.first == pmeta->autoinc)
		{
			continue;
		}
		if (is_first != 0)
		{
			sql += " , ";
		}
		is_first = 1;

		sql += '`' + kv.first + '`';
		sql += " = ";
		sql += kv.second;
	}
	if (!pmeta->autoinc.empty())
	{
		if (is_first != 0)
		{
			sql += " , ";
		}
		sql += '`' + pmeta->autoinc + '`';
		sql += " = ";
		sql += pmeta->autoinc;
		sql += "+1";
	}
	//where
	sql += " WHERE ";
	is_first = 0;
	for (auto & kv : kvlist)
	{
		is_pk = 0;
		for (auto & pk : pmeta->keys_names)
		{
			if (kv.first == pk)
			{
				is_pk = true;
				break;
			}
		}
		if (!is_pk)
		{
			continue;
		}
		if (is_first != 0)
		{
			sql += " AND ";
		}
		is_first = 1;

		sql += '`' + kv.first + '`';
		sql += " = ";
		sql += kv.second;
	}
	sql += ";";
	return 0;
}
int			MySQLMsgMeta::Insert(std::string & sql, const google::protobuf::Message & msg_key, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	//INSERT INTO order_(...)	values
	//update order_()	set		where
	sql = "INSERT INTO `";
	sql += GetTableName(msg_key);
	sql += "` (";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(impl, msg_key, kvlist, flatmode);
	if (ret) {
		GLOG_ERR("error msg sql get kv list!");
		return ret;
	}
	for (int i = 0; i < (int)kvlist.size(); ++i)
	{
		if (i != 0)
		{
			sql += ",";
		}
		sql += "`";
		sql += kvlist[i].first;
		sql += "`";
	}
	if (!pmeta->autoinc.empty())
	{
		if (!kvlist.empty()) {
			sql += ",";
		}
		sql += "`";
		sql += pmeta->autoinc;
		sql += "`";
	}
	sql += ") VALUES (";
	//VALUES(vs)
	for (int i = 0; i < (int)kvlist.size(); ++i)
	{
		if (0 != i)
		{
			sql += ",";
		}
		sql += kvlist[i].second;
	}
	if (!pmeta->autoinc.empty())
	{
		sql += ",";
		sql += "0";
	}
	sql += ");";
	return 0;

	return 0;
}
int        MySQLMsgMeta::Count(std::string & sql, const google::protobuf::Message & msg_key, const char * where_ )
{
	sql = "SELECT COUNT(*) FROM `";
	sql += GetTableName(msg_key);
	sql += "` ";
	if (where_ && where_[0]) {
		sql += where_;
	}
	sql += ";";
	return 0;
}
int			MySQLMsgMeta::CreateTable(std::string & sql, const google::protobuf::Message & msg_key, bool flatmode) {
	return CreateTables(sql, msg_key.GetTypeName(), -1, flatmode);
}
int			MySQLMsgMeta::DropTable(std::string & sql, const google::protobuf::Message & msg_key){
	sql = "DROP TABLE IF EXISTS `";
	sql += GetTableName(msg_key);
	sql += "`;";
	return 0;
}

int			MySQLMsgMeta::GetMsgFromSQLRow(google::protobuf::Message & msg, const MySQLRow &  sql_row) {
	if (sql_row.num_fields <= 0) {
		//error number fields
		GLOG_ERR("errror number fields:%d " , sql_row.num_fields);
		return -1;
	}
	int ret = SetMsgSQLKVList(msg, sql_row);
	if (ret) {
		GLOG_ERR("set msg from sql table:%s field value error !", sql_row.table_name);
	}
	return 0;
}

