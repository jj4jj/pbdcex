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
#define TABLE_NAME_POSTFIX		("_")
#define TABLE_NAME_FLAT_FIELD_SEP		("__")
#define TABLE_REPEATED_FIELD_POSTFIX	("$")
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
const google::protobuf::Descriptor * GetMsgDescriptorImpl(MySQLMsgMetaImpl * impl, const std::string & name) {
	return impl->protometa.GetMsgDesc(name.data());
}
static inline int	CheckMsgValidImpl(MySQLMsgMetaImpl * impl, const std::string & name, bool root, bool flatmode) {
	EXTMessageMeta meta;
	const Descriptor * msg_desc = GetMsgDescriptorImpl(impl, name);
	if (!msg_desc) {
		cerr << "not found desc by name:" << name << endl;
		return -1;
	}
	if (meta.AttachDesc(msg_desc)) {
		//error parse from desc
		cerr << "error parse from desc " << endl;
		return -1;
	}
	if (root) {
		if (msg_desc->full_name().find(TABLE_NAME_POSTFIX) != string::npos) {
			cerr << "forbidan the db msg type name include :" << TABLE_NAME_POSTFIX << " type:" <<
				msg_desc->full_name() << endl;
			return -2;
		}
		if (meta.pks_name.empty()) {
			//not found the pk def
			cerr << "not found the pk def " << endl;
			return -2;
		}
		if (meta.pks_name.size() != meta.pks_fields.size()) {
			cerr << "meta pks size not match " << endl;
			return -3;
		}
	}
	//check field type
	for (auto & sfm : meta.sub_fields) {
		if (flatmode) {
			if (sfm.field_desc->is_repeated() && sfm.z_count <= 0 ) {
				cerr << "not db repeat field but no count define! error field :" << sfm.field_desc->name() << " count:" << sfm.f_count << endl;
				return -4;
			}
			if (sfm.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
				int ret = CheckMsgValidImpl(impl, sfm.field_desc->message_type()->name(), false, true);
				if (ret) {
					return -5;
				}
			}
		}
		else {
			if (sfm.field_desc->is_repeated()) {
				cerr << "found a repeat field in not a flatmode :" << sfm.field_desc->name() << endl;
				return -6;
			}
		}
	}
	if (root) {
		if (meta.m_divnum < 0) {
			//must be a integer 
			cerr << "meta divnum is error :" << meta.m_divnum << endl;
			return -7;
		}
		if (!meta.m_divkey.empty() && !msg_desc->FindFieldByName(meta.m_divkey)) {
			cerr << "msg :" << msg_desc->name() << " div key:" << meta.m_divkey << " not found !" << endl;
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
		if (CheckMsgValidImpl(impl, name, true, false)) {
			return nullptr;
		}
		impl->msg_meta_cache[name] = etm;
		return &impl->msg_meta_cache[name];
	}
	return &it->second;
}
int		MySQLMsgMeta::InitMeta(int n, const char ** path, int m, const char ** otherfiles) {
	if (impl->protometa.Init(path, n, otherfiles, m)) {
		cerr << "proto meta init error !" << endl;
		return -1;
	}
	auto filedesc = impl->protometa.LoadFile(impl->meta_file.c_str());
	if (!filedesc) {
		cerr << "proto meta load error !" << endl;
		return -2;
	}
	impl->package_name = filedesc->package();
	return 0;
}
static inline string	GetMsgTableNameImpl(const std::string & name, int idx) {
	string tbname = name;
	if (idx > 0) {
		tbname += TABLE_NAME_POSTFIX;
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
	if (pmeta->m_divnum > 0) {
		uint64_t ullspkey = stoull(dcs::protobuf_msg_field_get_value(msg, pmeta->m_divkey, 0).c_str());
		return ullspkey % pmeta->m_divnum;
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
		fprintf(stderr, "escaped string but msyql connection is null ! data:%p size:%d", data, datalen);
		result = "''";
		return -1;
	}
	result.append(impl->escaped_buffer.data());
	result.append("'");
	return 0;
}
/////////////////////////////////////////////////////////////////////////
static inline std::string		
GetMsgTypeNameFromTableName(MySQLMsgMetaImpl * impl, const std::string & table_name) {
	string::size_type deli = table_name.find_last_of(TABLE_NAME_POSTFIX);
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

static inline string			
GetRepeatedFieldLengthName(MySQLMsgMetaImpl * impl, const std::string & name){
	string str_field_name = name;
	str_field_name += TABLE_REPEATED_FIELD_POSTFIX;
	str_field_name += "count";
	return str_field_name;
}
static inline string			
GetRepeatedFieldName(MySQLMsgMetaImpl * impl, const std::string & name, int idx){
	string str_field_name = name;
	str_field_name += TABLE_REPEATED_FIELD_POSTFIX;
	str_field_name += to_string(idx);
	return str_field_name;
}
static inline bool		
IsRepeatedFieldLength(MySQLMsgMetaImpl * impl, const std::string & field_name, const std::string & key){
	return (key == GetRepeatedFieldLengthName(impl, field_name));
}
static inline int	
GetMsgFlatTableSQLFields(MySQLMsgMetaImpl * impl, const std::string & name, std::string & sql, const std::string & prefix){
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
			sql += "`" + prefix + GetRepeatedFieldLengthName(impl,field.field_desc->name()) + "` ";
			sql += "INT UNSIGNED NOT NULL";
			sql += ",\n";		
			for (int i ; i < field.z_count; ++i){
				//prefix
				string field_prefix = prefix + GetRepeatedFieldName(impl, field.field_desc->name(), i);
				if (field.field_desc->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE){
					if (GetMsgFlatTableSQLFields(impl, field.field_desc->message_type()->name(), sql,
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
			if (GetMsgFlatTableSQLFields(impl, field.field_desc->message_type()->name(), sql,
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
//implementaion
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int			MySQLMsgMeta::CreateTables(const std::string & name, std::string & sql,int idx , bool flatmode){
	auto * pmeta = GetMsgExMeta(name);
	if (!pmeta) {
		return -1;
	}
	const EXTMessageMeta & meta = *pmeta;
	//////////////////////////////////////////////////////////////////////////
	if (flatmode) {
		//template
		string table_name = GetTableNameImpl(impl, name, idx);
		sql = "CREATE TABLE IF NOT EXISTS `" + table_name + "` (";
		//append table sql fields type defines	
		GetMsgFlatTableSQLFields(impl, name, sql, "");

		string pks = "";
		for (auto & pk : meta.pks_name)
		{
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
		if (idx < 0 && meta.m_divnum > 0) {
			for (int i = 0; i < meta.m_divnum; ++i) {
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
	else {
		//template
		string table_name = GetTableNameImpl(impl,name, idx);
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
		if (idx < 0 && meta.m_divnum > 0) {
			for (int i; i < meta.m_divnum; ++i) {
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
};
static void fetch_msg_sql_kvlist(const string & name, const google::protobuf::Message & msg, int idx, int level, void *ud, protobuf_sax_event_type evt) {
	fetch_msg_sql_kvlist_ctx	* ctx = (fetch_msg_sql_kvlist_ctx*)ud;
	string key;
	switch (evt) {
		case BEGIN_MSG:
			if (ctx->is_in_array) {
				ctx->prefix.push_back(name+ TABLE_REPEATED_FIELD_POSTFIX +to_string(idx));
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
			strjoin(key,TABLE_NAME_FLAT_FIELD_SEP,ctx->prefix);
			if (ctx->is_in_array) {
				key += TABLE_REPEATED_FIELD_POSTFIX + to_string(idx);
			}
			ctx->kvlist->push_back(make_pair(key, protobuf_msg_field_get_value(msg, name, idx)));
		break;
	}
}

static inline int GetMsgSQLKVList(const Message & msg, MsgSQLKVList & kvlist, bool flatmode) {
	fetch_msg_sql_kvlist_ctx ctx;
	ctx.kvlist = &kvlist;
	if (flatmode) {
		protobuf_msg_sax("", msg, fetch_msg_sql_kvlist, &ctx, 0, false);
	}
	else {
		for (int i = 0;i < msg.GetDescriptor()->field_count(); ++i) {
			auto field = msg.GetDescriptor()->field(i);
			if (msg.GetReflection()->HasField(msg, field)) {
				kvlist.push_back(make_pair(field->name(), protobuf_msg_field_get_value(msg, field->name(), -1)));
			}
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
		int ret = GetMsgSQLKVList(msg_key, kvlist, flatmode);
		if (ret) {
			GLOG_ERR("error msg sql get kv list!");
			return ret;
		}
		for (auto & kv : kvlist)
		{
			is_pk = 0;
			for (auto & pk : pmeta->pks_name)
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
			sql += "`";
			sql += kv.first;
			sql += "`";
			sql += " = ";
			sql += kv.second;
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
		int ret = GetMsgSQLKVList(msg_key, kvlist, flatmode);
		if (ret) {
			GLOG_ERR("error msg sql get kv list!");
			return ret;
		}

		for (auto & kv : kvlist)
		{
			is_pk = 0;
			for (auto & pk : pmeta->pks_name)
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
int			MySQLMsgMeta::Replace(std::string & sql, const google::protobuf::Message & msg, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	sql = "REPLACE INTO `";
	sql += GetTableName(msg);
	sql += "` (";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(msg_key, kvlist, flatmode);
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
int			MySQLMsgMeta::Update(std::string & sql, const google::protobuf::Message & msg, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg.GetTypeName());
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
	sql += GetTableName(msg);
	sql += "` SET ";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(msg, kvlist, flatmode);
	if (ret) {
		GLOG_ERR("error msg sql get kv list!");
		return ret;
	}

	int is_first = 0;
	int is_pk = 0;
	for (auto & kv : kvlist)
	{
		is_pk = 0;
		for (int j = 0; j < (int)pmeta->pks_name.size(); ++j)
		{
			if (kv.first == pmeta->pks_name[j])
			{
				is_pk = true;
				break;
			}
		}
		if (is_pk || kv.first == pmeta->m_autoinc)
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
	if (!pmeta->m_autoinc.empty())
	{
		if (is_first != 0)
		{
			sql += " , ";
		}
		sql += '`' + pmeta->m_autoinc + '`';
		sql += " = ";
		sql += pmeta->m_autoinc;
		sql += "+1";
	}
	//where
	sql += " WHERE ";
	is_first = 0;
	for (auto & kv : kvlist)
	{
		is_pk = 0;
		for (auto & pk : pmeta->pks_name)
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
int			MySQLMsgMeta::Insert(std::string & sql, const google::protobuf::Message & msg, bool flatmode )
{
	auto pmeta = GetMsgExMeta(msg_key.GetTypeName());
	if (!pmeta) {
		GLOG_ERR("not found the meta info get type name:%s", msg_key.GetTypeName().c_str());
		return -1;
	}
	//INSERT INTO order_(...)	values
	//update order_()	set		where
	sql = "INSERT INTO `";
	sql += GetTableName(msg);
	sql += "` (";
	MsgSQLKVList kvlist;
	int ret = GetMsgSQLKVList(msg_key, kvlist, flatmode);
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
	if (!pmeta->m_autoinc.empty())
	{
		if (!kvlist.empty()) {
			sql += ",";
		}
		sql += "`";
		sql += pmeta->m_autoinc;
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
	if (!pmeta->m_autoinc.empty())
	{
		sql += ",";
		sql += "0";
	}
	sql += ");";
	return 0;

	return 0;
}
int        MySQLMsgMeta::Count(std::string & sql, const google::protobuf::Message & msg, const char * where_ )
{
	sql = "SELECT COUNT(*) FROM `";
	sql += GetTableName(msg);
	sql += "` ";
	if (where_ && where_[0]) {
		sql += where_;
	}
	sql += ";";
	return 0;
}
int			MySQLMsgMeta::CreateTable(std::string & sql, const google::protobuf::Message & msg, bool flatmode ){
	return CreateTables(sql, msg.GetTypeName(), -1, flatmode);
}
int			MySQLMsgMeta::DropTable(std::string & sql, const google::protobuf::Message & msg){
	sql = "DROP TABLE IF EXISTS `";
	sql += GetTableName(msg);
	sql += "`;";
	return 0;
}







