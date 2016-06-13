#pragma  once
#include <string>
#include <vector>

struct pbdcex_flat_cpp_opt_t {
	enum code_style{
		CS_LOWER_CASE,
		CS_CAMMEL_CASE,
		CS_PASCAL,
		CS_HUNGARIAN
	} cs;
	enum organazation_style {
		OS_INT, //integration
		OS_DIS	//distribution
	} os;
	bool			generate_aux_list;
	std::string		custom_ext_name;
	std::string		out_path;
	pbdcex_flat_cpp_opt_t(){
		cs = CS_LOWER_CASE;
		os = OS_DIS;
		generate_aux_list = false;
		custom_ext_name = "";
		out_path = ".";
	}
};

struct pbdcex_sql_opt_t {
	std::string		out_path;
	bool			flat_mode;
	pbdcex_sql_opt_t(){
		flat_mode = false;
		out_path = "./";
	}
};
struct pbdcex_meta_config_t {
	std::vector<std::string>  paths;
	std::vector<std::string>  files;
};
struct pbdcex_config_t {
	pbdcex_flat_cpp_opt_t	cpp;
	pbdcex_sql_opt_t		sql;
	pbdcex_meta_config_t	meta;
};

struct pbdcex_t;
pbdcex_t *	pbdcex_create(const pbdcex_config_t & conf);
void		pbdcex_destroy(pbdcex_t *);

int		pbdcex_generate_flat_cpp_header(pbdcex_t *, const char * msg);
int		pbdcex_generate_mysql_create(pbdcex_t *, const char * msg);
int		pbdcex_generate_mysql_alter(pbdcex_t *, const char * msg, int oldtag = -1);

